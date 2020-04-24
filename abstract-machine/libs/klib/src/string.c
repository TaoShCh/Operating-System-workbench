#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t ans=0;
  for(int i=0;;i++){
    if(s[i]=='\0') break;
    ans++;
  }
  return ans;
}

char *strcpy(char* dst,const char* src) {
  char *ans=dst;
  for(int i=0;;i++){
    dst[i]=src[i];
    if(src[i]=='\0') break;
  }
  return ans;
}

char* strncpy(char* dst, const char* src, size_t n) {
  char *ans=dst;
  for(size_t i=0;i<n;i++){
    dst[i]=src[i];
    if(src[i]=='\0') break;
  }
  return ans;
}

char* strcat(char* dst, const char* src) {
  char *ans=dst;
  size_t n=strlen(dst);
  for(int i=0;;i++){
    dst[n+i]=src[i];
    if(src[i]=='\0') break;
  }
  return ans;
}

int strcmp(const char* s1, const char* s2) {
  for(int i=0;;i++){
    if(s1[i]!=s2[i]){
        return (int)s1[i]-(int)s2[i];
    }
    if(s1[i]=='\0') return 0;
  }
  return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
  for(int i=0;i<n;i++){
    if(s1[i]>s2[i]) return 1;
    if(s1[i]<s2[i]) return -1;
    if(s1[i]=='\0') return 0;
  }
  return 0;
}

void* memset(void* v,int c,size_t n) {
  void *ans=v;
  for(size_t i=0;i<n;i++){
    *(uint8_t*) v=(uint8_t) c;
    v=(uint8_t*)v+1;
  }
  return ans;
}

void* memcpy(void* out, const void* in, size_t n) {
  char *ans=out;
  for(int i=0;i<n;i++){
    *(uint8_t*)out=*(uint8_t*)in;
    out=(uint8_t*)out+1;
    in=(uint8_t*)in+1;
  }
  return ans;
}

int memcmp(const void* s1, const void* s2, size_t n){
  uint8_t temp=0;
  for(int i=0;i<n;i++){
    temp=*(uint8_t*)s1 - *(uint8_t*)s2;
    if(temp){
        return (int)temp;
    }
  }
  return 0;
}

void *memmove(void *dest, const void *src, size_t n){
    char *_dest = dest;
    const char *_src = src;
    if(_dest < _src){
        for(int i = 0; i < n; i++){
            _dest[i] = _src[i];
        }
    }
    else if(_dest > _src){
        for(int i = n - 1; i >= 0; i--){
            _dest[i] = _src[i];
        }
    }
    return dest;
}

#endif
