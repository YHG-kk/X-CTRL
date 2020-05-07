#include "DisplayPrivate.h"

PageManager page(PAGE_MAX);

#define PAGE_REG(name)\
do{\
    extern void PageRegister_##name(uint8_t pageID);\
    PageRegister_##name(PAGE_##name);\
}while(0)

void Page_Init()
{
    PAGE_REG(Home);
    PAGE_REG(CtrlPage);
    PAGE_REG(HandshakeAuto);
    PAGE_REG(Handshake);
    PAGE_REG(Scanner);
    PAGE_REG(RadioCfg);
    PAGE_REG(ChannelCfg);
    PAGE_REG(ChannelRevCfg);
    PAGE_REG(GyroscopeCfg);
    
    page.PagePush(PAGE_Home);
}

void Page_Delay(uint32_t ms)
{
    uint32_t lastTime = millis();
    while(millis() - lastTime <= ms)
    {
        lv_task_handler();
    }
}

void Page_ReturnHome()
{
    page.PageStackClear();
    page.PagePush(PAGE_Home);
}