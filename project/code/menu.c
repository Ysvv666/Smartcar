#include "menu.h"
static menu_item CarGo_Menu;
static menu_item DataParameter_Menu;
static menu_item DataStatusMenu;
static menu_item ImageMenu;
static menu_item Pa_Speed_Menu;
static menu_item Pa_Servo_Menu;
static menu_item Pa_Pid_Menu;
static menu_item St_Speed_Menu;
static menu_item St_Servo_Menu;
static menu_item St_Pid_Menu;
static menu_item main_Menu;

static char* main_menu_items[] = {"CarGo", "DataParameter", "DataStatus", "Image"};
static char* cargomenu_items[] = {"Start", "End"};
static char* dataparameter_items[] = {"Speed", "Servo", "PID"};
static char* datastatus_items[] = {"Speed", "Servo", "PID"};
static char* imagemenu_items[] = {"ImageRaw", "ImageYZ", "Variable"};
static char* paspeed_items[] = {"LineSpeed", "TurnSpeed", "Limit"};
static char* paservo_items[] = {"InitPwm", "LeftLimit", "RightLimit"};
static char* papid_items[] = {"SKp", "SKi", "SKd", "SMax_Iout", "SMax_out"};
static char* stspeed_items[] = {"LineSpeed", "TurnSpeed", "Limit"};
static char* stservo_items[] = {"InitPwm", "LeftLimit", "RightLimit"};
static char* stpid_items[] = {"SKp", "SKi", "SKd", "SMax_Iout", "SMax_out"};

static menu_item* DataParameter_child[] = {&Pa_Speed_Menu, &Pa_Servo_Menu, &Pa_Pid_Menu};
static menu_item* DataStatus_child[] = {&St_Speed_Menu, &St_Servo_Menu, &St_Pid_Menu};
static menu_item* main_child[] = {&CarGo_Menu, &DataParameter_Menu, &DataStatusMenu, &ImageMenu};


static menu_item CarGo_Menu = {
    .name = "CarGo",
    .content = cargomenu_items,
    .number = 2,
    .child = NULL
};

static menu_item Pa_Speed_Menu = {
    .name = "PaSpeed",
    .content = paspeed_items,
    .number = 3,
    .child = NULL
};

static menu_item Pa_Servo_Menu = {
    .name = "PaServo",
    .content = paservo_items,
    .number = 3,
    .child = NULL
};

static menu_item Pa_Pid_Menu = {
    .name = "PaPid",
    .content = papid_items,
    .number = 5,
    .child = NULL
};

static menu_item DataParameter_Menu = {
    .name = "DataParameter",
    .content = dataparameter_items,
    .number = 3,
    .child = DataParameter_child
};

static menu_item St_Speed_Menu = {
    .name = "StSpeed",
    .content = stspeed_items,
    .number = 3,
    .child = NULL
};

static menu_item St_Servo_Menu = {
    .name = "StServo",
    .content = stservo_items,
    .number = 3,
    .child = NULL
};

static menu_item St_Pid_Menu = {
    .name = "StPid",
    .content = stpid_items,
    .number = 5,
    .child = NULL
};

static menu_item DataStatusMenu = {
    .name = "DataStatus",
    .content = datastatus_items,
    .number = 3,
    .child = DataStatus_child
};

static menu_item ImageMenu = {
    .name = "Image",
    .content = imagemenu_items,
    .number = 3,
    .child = NULL
};

static menu_item main_Menu = {
    .name = "MainMenu",
    .content = main_menu_items,
    .number = 4,
    .child = main_child
};


static menu_item* currentMenu = &main_Menu;
static int cursor = 1;
static int currentIndex = 0;

void moveup(void) {
    if (cursor > 1) {
        cursor--;
    } else {
        cursor = currentMenu->number;
    }
    currentIndex = cursor - 1;
}

void movedown(void) {
    if (cursor < currentMenu->number) {
        cursor++;
    } else {
        cursor = 1;
    }
    currentIndex = cursor - 1;
}



void Sure(void) {
    if (currentMenu->child != NULL && currentMenu->child[currentIndex] != NULL) {
        currentMenu = currentMenu->child[currentIndex];
        cursor = 1;
        currentIndex = 0;
    }
}


void Back(void) {
    menu_item *parent = NULL;
    int parentIndex = 0;
    
    if (currentMenu == &CarGo_Menu) {
        parent = &main_Menu;
        parentIndex = 0; 
    } 
    else if (currentMenu == &DataParameter_Menu) {
        parent = &main_Menu;
        parentIndex = 1; 
    } 
    else if (currentMenu == &DataStatusMenu) {
        parent = &main_Menu;
        parentIndex = 2; 
    } 
    else if (currentMenu == &ImageMenu) {
        parent = &main_Menu;
        parentIndex = 3;  
    } 
    else if (currentMenu == &Pa_Speed_Menu) {
        parent = &DataParameter_Menu;
        parentIndex = 0; 
    } 
    else if (currentMenu == &Pa_Servo_Menu) {
        parent = &DataParameter_Menu;
        parentIndex = 1;  
    } 
    else if (currentMenu == &Pa_Pid_Menu) {
        parent = &DataParameter_Menu;
        parentIndex = 2;  
    } 
    else if (currentMenu == &St_Speed_Menu) {
        parent = &DataStatusMenu;
        parentIndex = 0;  
    } 
    else if (currentMenu == &St_Servo_Menu) {
        parent = &DataStatusMenu;
        parentIndex = 1;  
    } 
    else if (currentMenu == &St_Pid_Menu) {
        parent = &DataStatusMenu;
        parentIndex = 2; 
    }
    
    if (parent != NULL) {
        currentMenu = parent;
        cursor = parentIndex + 1; 
        currentIndex = parentIndex;
    }
}

void menu_display(void) {
    int i;
    char buffer[50];
    
    ips200_clear();

    sprintf(buffer, "Menu: %s", currentMenu->name);
    ips200_show_string(10, 10, buffer);

    for (i = 0; i < currentMenu->number; i++) {
        if (i == currentIndex) {
            sprintf(buffer, "-> %s", currentMenu->content[i]);
        } else {
            sprintf(buffer, "   %s", currentMenu->content[i]);
        }
        ips200_show_string(10, 30 + i * 20, buffer);
    }
}    



