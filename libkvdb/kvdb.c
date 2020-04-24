#include "kvdb.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void readflock(int fd){
    struct flock myflock;
    myflock.l_start = 0;
    myflock.l_len = 0;
    myflock.l_pid = getpid();
    myflock.l_type = F_RDLCK;
    myflock.l_whence = SEEK_SET;
    fcntl(fd, F_SETLK, myflock);
}

void writeflock(int fd){
    struct flock myflock;
    myflock.l_start = 0;
    myflock.l_len = 0;
    myflock.l_pid = getpid();
    myflock.l_type = F_WRLCK;
    myflock.l_whence = SEEK_SET;
    fcntl(fd, F_SETLKW, myflock);
}

void unlock(int fd){
    struct flock myflock;
    myflock.l_start = 0;
    myflock.l_len = 0;
    myflock.l_pid = getpid();
    myflock.l_type = F_UNLCK;
    myflock.l_whence = SEEK_SET;
    fcntl(fd, F_SETLKW, myflock);
}


int kvdb_open(kvdb_t *db, const char *filename){
    assert(pthread_mutex_init(&(db->mutex), NULL) == 0);
    assert(pthread_mutex_lock(&(db->mutex)) == 0);
    db->fp = fopen(filename, "a+");
    db->fd = fileno(db->fp);
    db->state = DB_VALID;
    if(db->fd < 0) return -1;
    readflock(db->fd);
    writeflock(db->fd);
    assert(pthread_mutex_unlock(&(db->mutex)) == 0);
    return 0;
}

int kvdb_close(kvdb_t *db){
    assert(pthread_mutex_lock(&(db->mutex)) == 0);
    if(db->state != DB_VALID){
        perror("db is not valid");
        return -1;
    }
    db->state  = 0;
    unlock(db->fd);
    close(db->fd);
    assert(pthread_mutex_unlock(&(db->mutex)) == 0);
    return 0;
}

int kvdb_put(kvdb_t *db, const char *key, const char *value){
    assert(pthread_mutex_lock(&(db->mutex)) == 0);
    if(db->state != DB_VALID){
        perror("db is not valid");
        assert(pthread_mutex_unlock(&(db->mutex)) == 0);
        return -1;
    }
    char this_key[MAX_STR] = "\0", this_value[MAX_STR] = "\0";
    strcpy(this_key, key); strcpy(this_value, value);
    strcat(this_key, "\n"); strcat(this_value, "\n");
    write(db->fd, "----begin----\n",14);
    write(db->fd, this_key, strlen(this_key));
    write(db->fd, this_value, strlen(this_value));
    write(db->fd, "----end----\n",12);
    assert(pthread_mutex_unlock(&(db->mutex)) == 0);
    return 0;
}


char this_value[MAX_STR];
char *kvdb_get(kvdb_t *db, const char *key){
    assert(pthread_mutex_lock(&(db->mutex)) == 0);
    memset(this_value, 0, sizeof(this_value));
    if(db->state != DB_VALID){
        assert(pthread_mutex_unlock(&(db->mutex)) == 0);
        perror("db is not valid");
        return NULL;
    }
    assert(fseek(db->fp, 0, SEEK_SET) == 0);
    char this_key[MAX_STR] = "\0";
    while(fgets(this_key, MAX_STR, db->fp) != NULL){
        this_key[strlen(this_key) - 1] = '\0';
        if(strcmp(this_key, key) == 0){
            fgets(this_value, MAX_STR, db->fp);
            this_value[strlen(this_value) - 1] = '\0';
            assert(pthread_mutex_unlock(&(db->mutex)) == 0);
            return this_value;
        }
    }
    assert(pthread_mutex_unlock(&(db->mutex)) == 0);
    return NULL;
}

