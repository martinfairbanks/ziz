#include "ziz.h"

Texture sprite;
void setup()
{
    createWindow(960, 540, false, "ZiZ");
    //memoryMapStruct(gameState, GameState);
    
    //sprite = loadBitmap("data/pix/invader.bmp");
    //   test = memoryPushStruct(Test);
    
}
void onLoad()
{}
void updateAndDraw()
{
    clear(colorTeal);
    //drawTexture(100.f, 5.f, &sprite);
    drawFilledRectToBuffer(&state->backBuffer,0.f, 0.f, 10.f, 10.f, colorGreen);
    pixel(210.f, 210.f, colorYellow);
    drawLine(0.f, 0.f, 300.f, 300.f, colorRed);
    drawCircle((f32)state->windowWidth/2.f, (f32)state->windowHeight/2.f, 100.f, colorYellow);
    drawFilledCircle((f32)state->windowWidth/2.f, (f32)state->windowHeight/2.f, 80.f, colorGreen);
    drawFilledCircle2((f32)state->windowWidth/2.f, (f32)state->windowHeight/2.f, 40.f, colorRed);
    drawTriangle(50.f, 100.f, 50.f, 150.f, 150, 150, colorWhite);
    v4 col = getPixel((f32)state->windowWidth/2.f, (f32)state->windowHeight/2.f);
    drawFilledTriangle(100+50, 100+50, 50+50, 150+50, 150+50, 150+50, col);
    drawRect(300, 300, 60, 60, colorGreen);
    drawFilledRect((f32)input->mouseX-25.f, (f32)input->mouseY-25.f, 50.f, 50.f, v4(1.0f,0.0f,0.0f,.7f));
    
}

void cleanUp()
{
    //textureFree(&sprite);
}