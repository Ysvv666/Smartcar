#ifndef _menu_h_
#define _menu_h_

#include "zf_common_headfile.h"

typedef struct menu_item {
    char* name;                
    char** content;            
    int number;                
    struct menu_item** child;  
} menu_item;

void menu_display(void);
void moveup(void);
void movedown(void);
void Sure(void);
void Back(void);


#endif
