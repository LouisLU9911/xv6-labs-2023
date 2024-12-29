// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 19
#define BULOCKNAMEBUFSZ 16

struct {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct spinlock bulocks[NBUCKET];

  struct buf buckets[NBUCKET];
} bcache;

char bucketname[NBUCKET][BULOCKNAMEBUFSZ];

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  for (int i = 0; i < NBUCKET; i++) {
    snprintf(bucketname[i], BULOCKNAMEBUFSZ, "bcache.bucket%d", i);
    initlock(&bcache.bulocks[i], bucketname[i]);
  }

  int i = 0;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    int bu_no = i % NBUCKET;
    initsleeplock(&b->lock, "buffer");
    acquire(&bcache.bulocks[bu_no]);
    b->next = bcache.buckets[bu_no].next;
    bcache.buckets[bu_no].next = b;
    release(&bcache.bulocks[bu_no]);
    i++;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b, *tmp;

  int bu_no = blockno % NBUCKET;

  // Is the block already cached?
  acquire(&bcache.bulocks[bu_no]);
  for(b = bcache.buckets[bu_no].next; b != 0; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bulocks[bu_no]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // 1. try to find an unused buf from the current bucket
  for(b = bcache.buckets[bu_no].next; b != 0; b = b->next){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.bulocks[bu_no]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.bulocks[bu_no]);

  // 2. search all buckets for an unused buf
  acquire(&bcache.lock);
  for (int i = 0; i < NBUCKET; i++) {
    acquire(&bcache.bulocks[i]);
    for(b = &bcache.buckets[i]; b->next != 0; b = b->next){
      tmp = b->next;
      if(tmp->refcnt == 0) {
        tmp->dev = dev;
        tmp->blockno = blockno;
        tmp->valid = 0;
        tmp->refcnt = 1;
        // move this buf out and release its bucket lock
        b->next = tmp->next;
        release(&bcache.bulocks[i]);
        goto foundbuf;
      }
    }
    release(&bcache.bulocks[i]);
  }
  release(&bcache.lock);
  panic("bget: no buffers");

foundbuf:
  acquire(&bcache.bulocks[bu_no]);
  tmp->next = bcache.buckets[bu_no].next;
  bcache.buckets[bu_no].next = tmp;
  release(&bcache.bulocks[bu_no]);
  release(&bcache.lock);
  acquiresleep(&tmp->lock);
  return tmp;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  b->refcnt--;
  releasesleep(&b->lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


