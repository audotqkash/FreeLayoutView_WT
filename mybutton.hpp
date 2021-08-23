#include <Arduino.h>
#include <LovyanGFX.hpp>
#define MYBUTTON_NORMAL   0x01
  #define MYBTNNML_WIDTH  60
  #define MYBTNNML_HEIGHT 40
#define MYBUTTON_SMALL    0x02
#define MYBUTTON_VER_TWO  0x05
#define MYBUTTON_CIRCLE_L 0x11
#define MUBUTTON_CIRCLE_S 0x12

struct myButton
{
  char    name[128] = {0};
  uint8_t type     = MYBUTTON_NORMAL;
  uint8_t grid_x   = 0;
  uint8_t grid_y   = 0;
  bool    focus    = false;
  bool    movable  = false;
  void  (*func)(char *) = NULL;
  bool pressed = false;
  float bgmin = 0;
  float bgmax = 0;
  float bgchg = 0;
  float bglvl = 0;


  myButton(){

  };

  myButton(const char *name){
      setName(name);
  }


  /**
   * @brief 
   * @param type[in] 
   *        - MYBUTTON_NORMAL   四角ボタン
   *        - MYBUTTON_VER_TWO  縦長2functionボタン
   * @param x 配置位置(水平)
   * @param y 配置位置(垂直)
   */
  void set(uint8_t type, uint8_t x, uint8_t y){
    this->grid_x = x;
    this->grid_y = y;
  }


  /**
   * @brief ボタン名設定
   */ 
  void setName(const char *txt)
  {
      strncpy(name, txt, 127);
      Serial.printf("Add \"%s\"\n", name);
  }

  void setForcus(bool val){
    this->focus = val;
  }

  void remove(){
    delete this;
  }

  uint8_t getWidth()
  {
    switch (this->type)
    {
    case MYBUTTON_NORMAL:
      return MYBTNNML_WIDTH;
    
    default:
      return 10;
    }  
  }

  uint8_t getHeight()
  {
    switch (this->type)
    {
        case MYBUTTON_NORMAL:
            return MYBTNNML_HEIGHT;
        default:
            return 10;
    }
  }

  void drawBacklight(int cor, LovyanGFX *base)
  {
    _drawBacklight(cor, base);
  }

  void clearBacklight(int cor, LovyanGFX *base)
  {
    _clearBacklight(cor, base);
  }

  void draw(int cor, LovyanGFX *base)
  {
    switch(type)
    {
      case MYBUTTON_NORMAL:
        _drawNormalButton((1 + grid_x) * cor, ( 1 + grid_y) * cor,
                          "ON", base);
        break;
        default:
          Serial.println("not suppert\n");
    }
  }

  void draw(int cor, LovyanGFX *base, uint16_t color)
  {
    switch(type)
    {
      case MYBUTTON_NORMAL:
        _drawNormalButton((1 + grid_x) * cor, ( 1 + grid_y) * cor,
                          "ON", color, base);
        break;
        default:
          Serial.println("not suppert\n");
    }
  }

  void setBacklight(float min, float size, float chg)
  {
    if(size <=0 || min <= 0){
      return;
    }
    bgmax = size;
    bgmin = min;
    bgchg = chg;
    bglvl = min;
    Serial.printf("%s : ", this->name);
    Serial.print(this->bgmax);Serial.print(" ");
    Serial.print(this->bgmin);Serial.print(" ");
    Serial.print(this->bglvl);Serial.print(" ");
    Serial.println();
  }

  void update()
  {
    /* バックライト更新*/
    if(bglvl >= bgmin && bglvl <= bgmax){
      bglvl += bgchg;
    }else{
      bgchg *= -1;
      bglvl += bgchg;
    }
    /*
    Serial.printf("%s \n", this->name);
    Serial.print(this->bglvl);Serial.print(" ");
    Serial.println();
    */
  }

  private:
  LGFX_Sprite sprite; // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。
  void _drawNormalButton(uint16_t x, uint16_t y, const char* label, LovyanGFX *base){

    _drawNormalButton(x, y, label, TFT_LIGHTGRAY, base);
  };

  void _drawNormalButton(uint16_t x, uint16_t y, const char* label, uint16_t color, LovyanGFX *base)
  {
    // createSpriteで幅と高さを指定してメモリを確保します。
    // 消費するメモリは色深度と面積に比例します。大きすぎるとメモリ確保に失敗しますので注意してください。
    _drawRectangle(MYBTNNML_WIDTH, MYBTNNML_HEIGHT, color, &sprite);

    sprite.setTextColor(TFT_BLACK, color);
    sprite.drawString(label, 15,10);
    // 作成したスプライトはpushSpriteで任意の座標に出力できます。
    // 出力先はインスタンス作成時に引数で渡したLGFXになります。
    sprite.pushSprite(base, x, y);        // lcdの座標64,0にスプライトを描画
  };

  void _drawRectangle(uint16_t w, uint16_t h, uint16_t color, LGFX_Sprite *spr)
  {
    spr->deleteSprite();
    spr->setColorDepth(16);
    spr->createSprite(w, h); // 幅65、高さ65でスプライトを作成。
    /*
    for (uint32_t x = 3; x < w-2; ++x) {
      for (uint32_t y = 3; y < h-2; ++y) {
        sprite.drawPixel(x, y, lcd.color888(3 + x*4, (x + y)*2, 3 + y*4));  // スプライトに描画
      }
    }
    */
    spr->fillScreen(color);

    int groove = min(w/10, h/10);
    groove = min(groove, 3);

    /* ボタンの溝 */
    for(int i = 0 ; i < groove; i++){
      spr->drawRect(i, i, w - (2*i), h - (2*i), LGFX_Sprite::color565(255 * i / groove, 255 * i / groove, 255 * i / groove));
    }
  };



  void _clearBacklight(int gridcor, LovyanGFX *base)
  {
    uint16_t x = (1 + this->grid_x) * gridcor;
    uint16_t y = (1 + this->grid_y) * gridcor;
    int h = this->getHeight();
    int w = this->getWidth();

    float max = this->bgmax;

    //int defaultx = ((w * max)-w) / 2;
    //int defaulty = ((h * max)-h) / 2;

    sprite.deleteSprite();
    sprite.createSprite(w * max, h * max); // 幅65、高さ65でスプライトを作成。

    sprite.fillScreen(TFT_BLACK);

    sprite.pushSprite(base, x + (w - w * max) / 2, y + (h - h * max) / 2);
    sprite.deleteSprite();
  }

  void _drawBacklight(int gridcor, LovyanGFX *base)
  {
    uint16_t x = (1 + this->grid_x) * gridcor;
    uint16_t y = (1 + this->grid_y) * gridcor;
    int h = this->getHeight();
    int w = this->getWidth();

    float max = this->bgmax;
    float current = this->bglvl;

    int defaultx = ((w * max)-w) / 2;
    int defaulty = ((h * max)-h) / 2;
    
    sprite.deleteSprite();
    sprite.createSprite(w * max, h * max); 

    sprite.fillScreen(TFT_BLACK);
    sprite.fillRect(defaultx, defaulty, w, h , TFT_WHITE);
    //lcd.fillRect(x + ((w - w * max) / 2),y + ((h - h * max) / 2),w * max,h * max, TFT_BLACK);

    int cnt = 0;
    for(float zoom = 0.8 ; zoom < current; zoom += 0.01)
    {
      cnt += 1;
      int wi = w * zoom;
      int hi = h * zoom;
      int color =  LovyanGFX::color565(255 * (1 - (zoom - 0.8)/(1.4-0.8)),255 * (1 - (zoom - 0.8)/(1.4-0.8)), 255 * (1 - (zoom - 0.8)/(1.4-0.8)));
      //lcd.drawRoundRect(x + ((w - wi) / 2),y + ((h - hi) / 2), wi, hi, 10, color);

      sprite.drawRoundRect(defaultx + ((w - wi) / 2), defaulty + ((h - hi) / 2), wi, hi, 10, color);
    }
    sprite.pushSprite(base, x + (w - w * max) / 2,y + (h - h * max) / 2);
    sprite.deleteSprite();
  }
};



