#include <common.h>
#include <klib.h>
#include <lock.h>

#define MAX_CPU 16

typedef struct task_node{
    task_t *task;
    struct task_node *next;
}task_node;
task_node *task_list_head;

struct _cpu{
    int ncli;
    int IF;
    task_node *current_task;
}mycpu[MAX_CPU];

int total_task;
spinlock_t task_lock;
spinlock_t context_lock;

void panic(char *msg){
    printf("%s", msg);
    _halt(1);
}

task_node *get_task_by_offset(int position){
    int x = position;
    task_node *cur = task_list_head;
    while (cur != NULL && x > 0){
        cur = cur->next;
        x--;
    }
    return cur;
}

_Context *context_save(_Event ev, _Context *context){
    kmt->spin_lock(&context_lock);
    task_node *cur = mycpu[_ncpu()].current_task;
    if(cur != NULL){
        cur->task->context = *context;
    }
    kmt->spin_unlock(&context_lock);
    return NULL;
}

_Context *context_switch(_Event ev, _Context *context){
    kmt->spin_lock(&context_lock);
    task_node *cur = mycpu[_ncpu()].current_task;
    cur->task->state = FREE;
    if(task_list_head == NULL){
        panic("No task to switch");
    }
    _Context *ans = NULL;
    while(1){
        if(cur == NULL){
            cur =task_list_head;
            while(1){
                if(cur == NULL){
                    panic("No task to run");
                }
                if(cur->task->state == FREE){
                    cur->task->state = RUNNING;
                    mycpu[_ncpu()].current_task = cur;
                    ans = &cur->task->context;
                    break;
                }
                cur = cur->next;
            }
            break;
        }
        if(cur->task->state == FREE){
            cur->task->state = RUNNING;
            mycpu[_ncpu()].current_task = cur;
            ans = &cur->task->context;
            break;
        }
        cur = cur->next;
    }
    kmt->spin_unlock(&context_lock);
    return ans;
}

static void kmt_init(){
    for(int i = 0; i < MAX_CPU; i++){
        mycpu[i].IF = mycpu[i].ncli = 0;
        mycpu[i].current_task = NULL;
    }
    task_list_head = NULL;
    total_task = 0;
    kmt->spin_init(&task_lock, "task_lock");
    kmt->spin_init(&context_lock, "context_lock");
    os->on_irq(-99999, _EVENT_NULL, context_save);
    os->on_irq(99999, _EVENT_NULL, context_switch);
}

static int create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    task_t *new_task = pmm->alloc(sizeof(task_t));
    new_task->name = name;
    new_task->entry = entry;
    new_task->arg = arg;
    new_task->state = FREE;
    new_task->stack = pmm->alloc(STACK_SIZE);
    new_task->context = *_kcontext((_Area){task->stack, task->stack + STACK_SIZE - 1}, entry, arg);
    if(new_task->stack == NULL) {
        panic("insufficient space for the thread");
    }
    kmt->spin_lock(&task_lock);
    task_node *new_node = pmm->alloc(sizeof(task_node));
    new_node->task = new_task;
    new_node->next =NULL;
    if(task_list_head == NULL){
        task_list_head = new_node;
    }
    else{
        task_node *last_node = task_list_head;
        while(last_node->next != NULL){
            last_node = last_node->next;
        }
        last_node->next = new_node;
    }
    total_task++;
    kmt->spin_unlock(&task_lock);
    return 0;
}

static void teardown(task_t *task){
    //TODO
}

static void pushcli(){
    uint32_t eflags;
    eflags = get_efl();
    cli();
    if(mycpu[_cpu()].ncli == 0){
        mycpu[_cpu()].IF = eflags & FL_IF;
    }
    mycpu[_cpu()].ncli += 1;
}

static void popcli(){
    if(get_efl() & FL_IF){
        panic("popcli while IF = 1\n");
    }
    if(--mycpu[_cpu()].ncli < 0){
        panic("IF < 0 after popcli\n");
    }
    if(mycpu[_cpu()].ncli == 0 && mycpu[_cpu()].IF != 0){
        sti();
    }
}

static int holding_lock(spinlock_t *lk){
    return lk->locked && lk->cpu == _cpu();
}

static void spin_init(spinlock_t *lk, const char *name){
    lk->name = name;
    lk->locked = 0;
    lk->cpu = -1;
}

static void spin_lock(spinlock_t *lk){
    pushcli();
    if(holding_lock(lk)){
        panic("spin_lock while already have acquired lock\n");
    }
    while(_atomic_xchg(&lk->locked, 1) != 0)
        ;
    lk->cpu = _cpu();
}

static void spin_unlock(spinlock_t *lk){
    if(!holding_lock(lk)){
        panic("spin_unlock while already have released lock\n");
    }
    lk->cpu = -1;
    _atomic_xchg(&lk->locked, 0);
    popcli();
}

static void sem_init(sem_t *sem, const char *name, int value){
    sem->name = name;
    sem->value = value;
    spin_init(&sem->lk, name);
}

static void sem_wait(sem_t *sem){
    spin_lock(&sem->lk);
    while(1){
        if(sem->value > 0){
            sem->value--;
            break;
        }
        spin_unlock(&sem->lk);
        _yield();
        spin_lock(&sem->lk);
    }

    spin_unlock(&sem->lk);
}

static void sem_signal(sem_t *sem){
    spin_lock(&sem->lk);
    sem->value++;
    spin_unlock(&sem->lk);
}

MODULE_DEF(kmt){
    .init = kmt_init,
    .create = create,
    .teardown = teardown,
    .spin_init = spin_init,
    .spin_lock = spin_lock,
    .spin_unlock = spin_unlock,
    .sem_init = sem_init,
    .sem_wait = sem_wait,
    .sem_signal = sem_signal,
};
