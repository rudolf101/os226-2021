#include <stddef.h>

#include "pool.h"

void pool_init(struct pool *p, void *mem, unsigned long nmemb, unsigned long membsz) {
    *p = POOL_INITIALIZER(mem, nmemb, membsz);
}

void *pool_alloc(struct pool *p) {
    struct pool_block *fb = p->free;
    if (fb) {
        p->free = fb->next;
        return fb;
    }
    if (p->free_start < p->free_end) {
        struct pool_block *r = p->free_start;
        p->free_start += p->membsz;
        return r;
    }
    return NULL;
}

void pool_free(struct pool *p, void *ptr) {
    struct pool_block *fb = ptr;
    *fb = (struct pool_block) {.next = p->free};
    p->free = fb;
}
