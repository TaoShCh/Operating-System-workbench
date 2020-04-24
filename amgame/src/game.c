#include <game.h>

void init_screen();
void splash();
void read_key();

int nowx,nowy,oldx,oldy;
int w,h;


int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  splash();
  while (1) {
    read_key();
  }
  return 0;
}

uint32_t color=0xffffff;
void read_key() {
  _DEV_INPUT_KBD_t event = { .keycode = _KEY_NONE };
  #define KEYNAME(key) \
    [_KEY_##key] = #key,
  static const char *key_names[] = {
    _KEYS(KEYNAME)
  };
  _io_read(_DEV_INPUT, _DEVREG_INPUT_KBD, &event, sizeof(event));
  if (event.keycode != _KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
  }
  oldx=nowx;oldy=nowy;
  int flag=1;
  switch (event.keycode){
    case 30:{
        nowy-=2;
        break;
    }
    case 45:{
        nowx+=2;
        break;
    }
    case 44:{
        nowy+=2;
        break;
    }
    case 43:{
        nowx-=2;
        break;
    }
    case 46:{
        color=(color+0x3)&0xffffff;
        if(color<0xf) color+=0xf;
        break;
    }
    default: {
        flag=0;
        break;
    }
  }
  if(flag) splash();
}



void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
  oldx=nowx=w/2-SIDE/2;
  oldy=nowy=h/2-SIDE/2;
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: allocated on stack
  _DEV_VIDEO_FBCTL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTL, &event, sizeof(event));
}



void splash() {
    draw_rect(oldx, oldy, SIDE, SIDE, 0x000000); // black
    draw_rect(nowx, nowy, SIDE, SIDE, color); // white
}
