#include "FileGroup.h"
#include "input.h"
#include "constants.h"

#ifdef USE_INPUT_PULLUP
#define INPUT_MODE INPUT_PULLUP
#define INPUT_STATE LOW
#else
#define INPUT_MODE INPUT
#define INPUT_STATE HIGH
#endif

void input_setup()
{
//  pinMode(K_LEFT, INPUT_MODE);
//  pinMode(K_RIGHT, INPUT_MODE);
//  pinMode(K_UP, INPUT_MODE);
//  pinMode(K_DOWN, INPUT_MODE);
//  pinMode(K_FIRE, INPUT_MODE);
}

bool input_left()
{
    return(CTRL.JS_R.X.Val < -200);
    //return digitalRead(K_LEFT) == INPUT_STATE;
};

bool input_right()
{
    return(CTRL.JS_R.X.Val > 200);
    //return digitalRead(K_RIGHT) == INPUT_STATE;
};

bool input_up()
{
    return(CTRL.JS_L.Y.Val > 200);
    //return digitalRead(K_UP) == INPUT_STATE;
};

bool input_down()
{
    return(CTRL.JS_L.Y.Val < -200);
    //return digitalRead(K_DOWN) == INPUT_STATE;
};

bool input_fire()
{
    return digitalRead(K_FIRE) == INPUT_STATE;
};
