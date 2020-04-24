#ifndef __KVDB_H__
#define __KVDB_H__

#include <pthread.h>
#include <stdio.h>

#define MAX_STR 1000
#define DB_VALID 1

struct kvdb {
    pthread_mutex_t mutex;
    int fd;
    FILE *fp;
    int state;
};
typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

#endif
