#include "FileGroup.h"
#include "FileSystem.h"
#include "GUI_Private.h"
#include "FifoQueue.h"
#include "wav_decoder.h"

/*实例化当前页面调度器*/
static MillisTaskManager mtm_WavPlayer(2);

String WavFilePath;
static bool lastState_BuzzSound;

#define WaveResolution  512
#define FIFO_Size   4096

static uint8_t WavBuffer[FIFO_Size];
static FifoQueue<uint8_t> WavFifo(WavBuffer, FIFO_Size);

static File wavFile;
static WAV_TypeDef wav;

extern "C" {
    void TIM2_IRQHandler()
    {
        TIM_ClearITPendingBit(PIN_MAP[Buzz_Pin].TIMx, TIM_IT_Update);
        if(WavFifo.available() < 8)
        {
            return;
        }
        else
        {
            Wav_Next_16Bit2Channel(&wav);
            int data = wav.CurrentData.LeftVal * 1 * WaveResolution / 2 / 32768;
            pwmWrite(Buzz_Pin, constrain(WaveResolution / 2 + data, 0, WaveResolution));
        }
    }
}

/*ProgressBar(T *obj, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t dir, uint8_t style)*/
static LightGUI::ProgressBar<SCREEN_CLASS> WavPrg(&screen, 5, screen.height() - 10, screen.width() - 10, 6, 0, 0);

static void Task_UpdateProgress()
{
    WavPrg.setProgress((float)wav.DataPosition/wav.DataSize);
    TextSetDefault();
    screen.setCursor(5, WavPrg.Y - TEXT_HEIGHT_2 - 2);
    int sec = wav.DataPosition / wav.Header.BytePerSecond;
    int min = sec / 60;
    screen.printfX("%02d:%02d ", min, sec % 60);
    
    if(wav.IsEnd)
        page.PageChangeTo(PAGE_FileExplorer);
}

static void Task_WavBufferUpdate()
{
    static char buffer[4];
    if(WavFifo.available() < FIFO_Size - 16)
    {
        wavFile.read(buffer, 4);
        for(int i = 0; i < 4; i++)
        {
            WavFifo.write(buffer[i]);
        }
    }
}

static char BUFFER[32];
static uint8_t Wav_TestInterface(HWAVEFILE handle, uint8_t size, uint8_t **buffer)
{
    int bufferPos = 0;
    while(size --)
    {
        BUFFER[bufferPos ++] = WavFifo.read();
    }
    *buffer = (uint8_t *)BUFFER;
    return 0;
}

static void Init_WaveTest()
{
    pinMode(Buzz_Pin, INPUT);
    lastState_BuzzSound = State_BuzzSound;
    State_BuzzSound = OFF;

    wavFile = SD.open(WavFilePath);

    if(!wavFile)
    {
        page.PageChangeTo(PAGE_FileExplorer);
        TextSetDefault();
        TextMidPrint(0.5f, 0.7f, "wav error!");
        PageDelay(200);
        return;
    }

    WavFifo.flush();
    while(WavFifo.available() < FIFO_Size - 16)
    {
        Task_WavBufferUpdate();
    }

    Wav_StructInit(&wav, Wav_TestInterface);
    Wav_Open(&wav);

    PWM_Init(Buzz_Pin, WaveResolution, 48000);
    TIM_Cmd(PIN_MAP[Buzz_Pin].TIMx, DISABLE);
    TIM_ITConfig(PIN_MAP[Buzz_Pin].TIMx, TIM_IT_Update, ENABLE);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM_ARRPreloadConfig(PIN_MAP[Buzz_Pin].TIMx, DISABLE);
    TIM_Cmd(PIN_MAP[Buzz_Pin].TIMx, ENABLE);
}

/********** 基本 ************/
/**
  * @brief  页面初始化事件
  * @param  无
  * @retval 无
  */
static void Setup()
{
    ClearPage();
    char filepath[20];
    WavFilePath.toCharArray(filepath, WavFilePath.length() + 1);
    TextSetDefault();
    TextMidPrint(0.5f, 0.5f, filepath);
    Init_WaveTest();
    
    mtm_WavPlayer.TaskRegister(0, Task_WavBufferUpdate, 0);
    mtm_WavPlayer.TaskRegister(1, Task_UpdateProgress, 400);
}

/**
  * @brief  页面循环事件
  * @param  无
  * @retval 无
  */
static void Loop()
{
    mtm_WavPlayer.Running(millis());
}

/**
  * @brief  页面退出事件
  * @param  无
  * @retval 无
  */
static void Exit()
{
    if(wavFile)
        wavFile.close();

    NVIC_DisableIRQ(TIM2_IRQn);
    TIM_Cmd(PIN_MAP[Buzz_Pin].TIMx, DISABLE);
    pinMode(Buzz_Pin, OUTPUT);
    digitalWrite(Buzz_Pin, LOW);
    State_BuzzSound = lastState_BuzzSound;
    ClearPage();
}

/************ 事件处理 **********/
/**
  * @brief  按键按下事件
  * @param  无
  * @retval 无
  */
static void ButtonPressEvent()
{
    if(btBACK)
    {
        page.PageChangeTo(PAGE_FileExplorer);
    }
}

/**
  * @brief  按键长摁事件
  * @param  无
  * @retval 无
  */
static void ButtonLongPressEvent()
{
}

/**
  * @brief  页面注册
  * @param  ThisPage:为此页面分配的ID号
  * @retval 无
  */
void PageRegister_WavPlayer(uint8_t ThisPage)
{
    /*基本*/
    page.PageRegister_Basic(ThisPage, Setup, Loop, Exit);

    /*事件*/
    page.PageRegister_Event(ThisPage, EVENT_ButtonPress, ButtonPressEvent);
    page.PageRegister_Event(ThisPage, EVENT_ButtonLongPress, ButtonLongPressEvent);
}
