#include <string.h>
#include <stdio.h>

#include "sched.h"
#include "pool.h"

static unsigned int time = 0;

struct sched_task {
    void (*entrypoint)(void *aspace);

    void *aspace;
    int priority;
    int deadline;
    int timeout;
};

struct sched_node {
    struct sched_task task;
    struct sched_node *next;
} sched_node_arr[16];

static struct sched_node *sched_head = NULL;

static struct pool sched_node_pool = POOL_INITIALIZER_ARRAY(sched_node_arr)

void add_task(struct sched_task task) {
    struct sched_node *node = pool_alloc(&sched_node_pool);
    if (node == NULL) return;
    node->task = task;
    node->next = sched_head;
    sched_head = node;
}

static struct sched_task *cur_task = NULL;

void run_tasks(int (*is_preferable)(struct sched_task old_task, struct sched_task new_task)) {
    while (sched_head != NULL) {
        struct sched_node *prev = NULL, *node = NULL;
        for (struct sched_node *cur_prev = NULL, *cur_node = sched_head;
             cur_node != NULL;
             cur_node = cur_node->next) {
            if ((node == NULL && cur_node->task.timeout <= time) ||
                (node != NULL && is_preferable(node->task, cur_node->task))) {
                prev = cur_prev;
                node = cur_node;
            }
            cur_prev = cur_node;
        }
        if (node != NULL) {
            if (prev != NULL) prev->next = node->next;
            else sched_head = node->next;
            cur_task = &(node->task);
            cur_task->entrypoint(cur_task->aspace);
            cur_task = NULL;
            pool_free(&sched_node_pool, node);
        }
    }
}

void sched_new(void (*entrypoint)(void *aspace),
               void *aspace,
               int priority,
               int deadline) {
    add_task((struct sched_task) {entrypoint, aspace, priority, deadline, 0});
}

void sched_cont(void (*entrypoint)(void *aspace),
                void *aspace,
                int timeout) {
    if (cur_task == NULL) return;
    add_task((struct sched_task) {entrypoint, aspace, cur_task->priority, cur_task->deadline, time + timeout});
}

int is_preferable_by_fifo(struct sched_task old_task, struct sched_task new_task) {
    return new_task.timeout <= time;
}

int is_preferable_by_prio(struct sched_task old_task, struct sched_task new_task) {
    return new_task.timeout <= time && new_task.priority >= old_task.priority;
}

int is_preferable_by_deadline(struct sched_task old_task, struct sched_task new_task) {
    return (0 < new_task.deadline && (new_task.deadline < old_task.deadline || old_task.deadline <= 0)) ||
           (new_task.deadline == old_task.deadline && is_preferable_by_prio(old_task, new_task));
}

void sched_time_elapsed(unsigned amount) {
    time += amount;
}

void sched_run(enum policy policy) {
    switch (policy) {
        case POLICY_FIFO:
            run_tasks(&is_preferable_by_fifo);
            break;
        case POLICY_PRIO:
            run_tasks(&is_preferable_by_prio);
            break;
        case POLICY_DEADLINE:
            run_tasks(&is_preferable_by_deadline);
            break;
        default:
            fprintf(stderr, "Unknown scheduler policy provided\n");
    }
}