#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char ans[64]={'\0'};

void int2str(int x){
    memset(ans, 0, 64);
    int i = 0, d = x;
    if(x == 0){
        *ans = '0';
        return;
    }
    else if(x < 0){
        d = -d;
        i = 1;
        ans[0] = '-';
    }
    int highest = 1;
    while(d / highest){
        highest *= 10;
    }
    highest /= 10;
    while(highest){
        ans[i]=(char)(d / highest + 48);
        d %= highest;
        highest /= 10;
        i++;
    }
  ans[i] = '\0';
}


int printf(const char *fmt, ...) {
  char out[128]={'\0'};
  int val,buf_len=0;
  char c,*s;
  va_list ap;
  va_start(ap,fmt);
  int numof0;
  while(*fmt){
    if(*fmt!='%'){
      memcpy(out+buf_len,fmt,1);
      buf_len++;fmt++;
      continue;
    }
    else fmt++;
    switch(*fmt){
      case 's':
        s=va_arg(ap,char *);
        strcpy(out+buf_len,s);
        buf_len+=strlen(s);
        break;
      case 'd':
        val=va_arg(ap,int);
        int2str(val);
        strcpy(out+buf_len,ans);
        buf_len+=strlen(ans);
        break;
      case 'c':
        c=va_arg(ap,int);
        out[buf_len]=c;
        buf_len++;
        break;
      case '0':
        numof0=0;
        fmt++;
        while(*fmt>=48&&*fmt<=57){
          numof0=numof0*10+*fmt-48;
          fmt++;
        }
        switch(*fmt){
          case 'd':
            val=va_arg(ap,int);
            int2str(val);
            numof0-=strlen(ans);
            char outwith0[512]={'\0'};
            memset(outwith0,48,numof0);

            strcat(outwith0,ans);
            strcpy(out+buf_len,outwith0);
            buf_len+=strlen(outwith0);
            break;
        }
        break;
      default:
        printf("error or some functions are missed.\n");break;
    }
    fmt++;
  }
  out[buf_len]='\0';
  va_end(ap);
  for(int i=0;out[i];i++){
    _putc(out[i]);
  }
  return buf_len;

}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

int sprintf(char *out, const char *fmt, ...) {
  int val,buf_len=0;
  int numof0;
  char c,*s,*buf=out;
  va_list ap;
  va_start(ap,fmt);
  while(*fmt){
    if(*fmt!='%'){
      memcpy(buf+buf_len,fmt,1);
      buf_len++;fmt++;
      continue;
    }
    else fmt++;
    switch(*fmt){
      case 's':
        s=va_arg(ap,char *);
        strcpy(buf+buf_len,s);
        buf_len+=strlen(s);
        break;
      case 'd':
        val=va_arg(ap,int);
        int2str(val);
        strcpy(buf+buf_len,ans);
        buf_len+=strlen(ans);
        break;
      case 'c':
        c=va_arg(ap,int);
        out[buf_len]=c;
        buf_len++;
        break;
      case '0':
        numof0=0;
        fmt++;
        while(*fmt>=48&&*fmt<=57){
          numof0=numof0*10+*fmt-48;
          fmt++;
        }
        switch(*fmt){
          case 'd':
            val=va_arg(ap,int);
            int2str(val);
            numof0-=strlen(ans);
            char outwith0[512]={'\0'};
            memset(outwith0,48,numof0);
            strcat(outwith0,ans);
            strcpy(buf+buf_len,outwith0);
            buf_len+=strlen(outwith0);
            break;
        }
        break;
      default:
        printf("error or some functions are missed.\n");break;
    }
    fmt++;
  }
  out[buf_len]='\0';
  va_end(ap);
  return buf_len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
