#include <common.h>
#include <klib.h>
#include <devices.h>
spinlock_t trap_lock;

typedef struct irq_handler_t{
    struct irq_handler_t *next;
    handler_t handler;
    int event;
    int seq;
}handler_node;
handler_node *handler_head;

void print_handler_list(){
    handler_node *print_node = handler_head;
    while(print_node != NULL){
        //printf("hello\n");
        printf("event:%d seq:%d\n", print_node->event, print_node->seq);
        print_node = print_node->next;
    }
}


void irq_test(){
    os->on_irq(0, 1, os->trap);
    os->on_irq(2, 1, os->trap);
    os->on_irq(4, 1, os->trap);
    os->on_irq(3, 1, os->trap);
    os->on_irq(-19, 1, os->trap);
    os->on_irq(2, 2, os->trap);
    os->on_irq(4, 2, os->trap);
    os->on_irq(1, 2, os->trap);
    os->on_irq(3, 2, os->trap);
    os->on_irq(2, 7, os->trap);
    //print_handler_list();
}


static void os_init() {
    pmm->init();
    kmt->init();
    handler_head = NULL;
    kmt->spin_init(&trap_lock, "trap_lock");
    printf("hello\n");
    //irq_test();
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]); _putc('\n');
}

static void os_run() {
  hello();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
    kmt->spin_lock(&trap_lock);
    _Context *ret = context;
    handler_node *this_handler = handler_head;
    while(this_handler != NULL){
        if(this_handler->event == _EVENT_NULL || this_handler->event == ev.event){
            _Context *next = this_handler->handler(ev, context);
            if(next != NULL){
                ret = next;
            }
            this_handler = this_handler->next;
        }
    }
    kmt->spin_unlock(&trap_lock);
    return ret;
}

static void add_handler_to_list(handler_node *node){
    handler_node *pivot = handler_head;
    if(handler_head == NULL){
        handler_head = node;
        return;
    }
    else{
        handler_node *last_pivot = NULL;
        while(pivot != NULL){
            if(pivot->event == node->event){
                //this event handler alreadt exists
                if(node->seq < pivot->seq){//this seq is the smallest
                    node->next = pivot;
                    if(last_pivot == NULL){
                        handler_head = node;
                    }
                    else last_pivot->next = node;
                    return;
                }
                while(pivot->event == node->event && pivot->next->event == node->event){
                    if(pivot->seq <= node->seq && pivot->next->seq >= node->seq){
                        node->next = pivot->next;
                        pivot->next = node;
                        return;
                    }
                    pivot = pivot->next;
                }
                //this seq is the biggest
                node->next = pivot->next;
                pivot->next = node;
                return;
            }
            last_pivot = pivot;
            pivot = pivot->next;
        }
        //this is a new event
        last_pivot->next = node;
        return;
    }
}

static void os_on_irq(int seq, int event, handler_t handler) {
    handler_node *new_node = pmm->alloc(sizeof(handler_node));
    new_node->event = event;
    new_node->seq = seq;
    new_node->handler = handler;
    new_node->next = NULL;
    add_handler_to_list(new_node);
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
