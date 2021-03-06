#include <Arduino.h>
#include <mybutton.hpp>
#include <LovyanGFX.hpp>
#include <wioMenuView.hpp>
#include <vector>

#define buttonLayout_MVU  1
#define buttonLayout_MVD  2
#define buttonLayout_MVR  3
#define buttonLayout_MVL  4

#define GRIDINTERVAL 10


class buttonLayout
{
    private:
        static void cancel()
        {
            Serial.printf("cancel!");
        }
        static void save()
        {
            Serial.printf("save!");
        }
        std::vector<myButton*> btns;
        LGFX_Sprite cambus;
        bool editmode = false;

        float backlight = 1.4;
        const float backlightchg = 0.08;

        /* MENU */
        MenuView menu;
        menuTree topmenu;
        menuTree editmenu;
        bool     menuOpened;

    public:
        buttonLayout(LovyanGFX *base)
        {
          cambus.setColorDepth(32);
          cambus.createSprite(base->width() - (30 * 2), base->height() - (30 * 2));
        }
        void reset(LovyanGFX *base){
          cambus.deleteSprite();
          cambus.deletePalette();
          cambus.createSprite(base->width() - (30 * 2), base->height() - (30 * 2));
          cambus.fillScreen(TFT_BLACK);
        }

        void end()
        {
          cambus.fillScreen(TFT_BLACK);
        }
        size_t add(myButton *btn){
            auto itr = btns.begin();
            btns.insert(itr, btn);
            return btns.size();
        }

        void begin()
        {
            pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
            pinMode(WIO_5S_LEFT, INPUT_PULLUP);
            pinMode(WIO_5S_DOWN, INPUT_PULLUP);
            pinMode(WIO_5S_UP, INPUT_PULLUP);
            pinMode(WIO_5S_PRESS, INPUT_PULLUP);
            pinMode(WIO_KEY_A, INPUT_PULLUP);
            pinMode(WIO_KEY_B, INPUT_PULLUP);
            pinMode(WIO_KEY_C, INPUT_PULLUP);

            if(btns.size() == 0)
            {
                return;
            }
            for(auto itr = btns.begin(); itr != btns.end(); ++itr)
            {
                if((*itr)->focus == true)
                {
                    return;
                }
            }
            
            menu.setMenuTree(topmenu);
            menu.setItem("save", &buttonLayout::save);
            menu.setItem("cancel",  &buttonLayout::cancel);
            topmenu = menu.getMenuTree();
            menuOpened = false;
            
            btns[0]->focus = true;
            return;
        }

        size_t getIterator(std::vector<myButton *>::iterator *begin, 
                         std::vector<myButton *>::iterator *end)
        {
            *begin = btns.begin();
            *end   = btns.end();
            return btns.size();
        }

        void changeMode()
        {
          if(editmode == false)
          {
            Serial.println("Edit mode");
          }else{
            Serial.println("Normal mode");
            cambus.fillScreen(TFT_BLACK);
          }
          editmode = !editmode;
        }

        myButton *getFocusedButton(){
          myButton *ptr;
          getFocusedButton(&ptr);
          return ptr;
        }
        void getFocusedButton(myButton **ptr)
        {
          for(auto vaitr = std::begin(btns); vaitr != std::end(btns); ++vaitr)
          {
            if((*vaitr)->focus == true){
              *ptr = *vaitr;
            }
          }
        }

        void locationChg(myButton *btn, int chgx, int chgy)
        {
          Serial.printf("current %d  %d\n", btn->grid_x, btn->grid_y);
          if((btn->grid_x + chgx) >= 0 && 
             (btn->grid_x + chgx + (btn->getWidth() / GRIDINTERVAL)) <= (cambus.width() / GRIDINTERVAL))
          {
            btn->grid_x += chgx;
          }
          if((btn->grid_y + chgy) >= 0 && 
             (btn->grid_y + chgy + (btn->getHeight() / GRIDINTERVAL)) <= (cambus.height() / GRIDINTERVAL))
          {
            btn->grid_y += chgy;
          }
        }

        void searchNearButtons(myButton **up, myButton **down, myButton **left, myButton **right)
        {
          /*       ????????????  / ?????????    ************************************************************/
          myButton *cur = NULL;
          int ldst = INT_MAX, rdst = INT_MAX, tdst = INT_MAX, ddst = INT_MAX;
          /*       ??????????????????????????????    ***********************************************************/
          getFocusedButton(&cur);

          if(cur == NULL){
            return;
          }

          *down = cur;
          *up   = cur;
          *left = cur;
          *right= cur;

          for(auto vaitr = std::begin(btns); vaitr != std::end(btns); ++vaitr)
          {
            int distd,distu, distr, distl, disth, distv;
            if(*vaitr == cur){
              continue;
            }
        
            /* ???????????????????????? */
            distd = ((*vaitr)->grid_y * GRIDINTERVAL) - ((cur->grid_y * GRIDINTERVAL) + cur->getHeight());
            distu = (cur->grid_y * GRIDINTERVAL) - ((*vaitr)->grid_y * GRIDINTERVAL);
            distv = abs(((*vaitr)->grid_y * GRIDINTERVAL)                                  /*vartical  */
                             - ((cur->grid_y * GRIDINTERVAL) + (cur->getHeight() / 2))) + 
                        abs((((*vaitr)->grid_y * GRIDINTERVAL) + (*vaitr)->getHeight()) 
                             - ((cur->grid_y * GRIDINTERVAL) + (cur->getHeight() / 2))); 
            /* ???????????????????????? */
            distr = ((*vaitr)->grid_x * GRIDINTERVAL) - ((cur->grid_x * GRIDINTERVAL) + cur->getWidth());
            distl = (cur->grid_x * GRIDINTERVAL) - ((*vaitr)->grid_x * GRIDINTERVAL);                      /*horizontal*/
            disth = abs(((*vaitr)->grid_x * GRIDINTERVAL) 
                     - ((cur->grid_x * GRIDINTERVAL) + (cur->getWidth() / 2))) + 
                    abs((((*vaitr)->grid_x * GRIDINTERVAL) + (*vaitr)->getWidth()) 
                     - ((cur->grid_x * GRIDINTERVAL) + (cur->getWidth() / 2))); 

            if(distu > 0 && (distu + disth * 10) < tdst){ /* ????????? */
              tdst = distu + disth * 10;
              *up = *vaitr;
            }
            if(distd > 0 && (distd + disth * 10) < ddst){ /* ????????? */
              ddst = distd + disth * 10;
              *down = *vaitr;
            }

            if(distr > 0 && (distr + distv * 10) < rdst){
              *right = *vaitr;
              rdst = distr + distv * 10;
            }

            if(distl > 0 && (distl + distv * 10) < ldst){
              *left = *vaitr;
              ldst = distl + distv * 10;
            }
//    Serial.printf("u(%d,%d) = %d,%d\n",(*vaitr)->grid_x, (*vaitr)->grid_y, distu, disth);
//    Serial.printf("d(%d,%d) = %d,%d\n",(*vaitr)->grid_x, (*vaitr)->grid_y, distd, disth);
//    Serial.printf("l(%d,%d) = %d,%d\n",(*vaitr)->grid_x, (*vaitr)->grid_y, distl, disth);
//    Serial.printf("r(%d,%d) = %d,%d\n",(*vaitr)->grid_x, (*vaitr)->grid_y, distr, disth);
          }
//  //Serial.printf("tdst = %d\n", tdst);
//  //Serial.printf("ddst = %d\n", ddst);
//  Serial.printf("ldst = %d\n", ldst);
//  Serial.printf("rdst = %d\n", rdst);
//  Serial.println("");
//
        }                                                        /*<  searchNearButtons  */

        void move(uint8_t direction)
        {
          if(menuOpened == true)
          {
            return;
          }
          switch(direction)
          {
            case buttonLayout_MVU:
              moveUp();
              break;
            case buttonLayout_MVD:
              moveDown();
              break;
            case buttonLayout_MVR:
              moveRight();
              break;
            case buttonLayout_MVL:
              moveLeft();
              break;
          }
        }        
        void moveUp()
        {
          myButton *dummy, *next;
          if(editmode == false || getFocusedButton()->movable == false){
            searchNearButtons(&next, &dummy, &dummy, &dummy);
            select(next);
          }else{
            locationChg(getFocusedButton(), 0, -1);
          }
        }
        void moveDown()
        {
          myButton *dummy, *next;
          if(editmode == false || getFocusedButton()->movable == false){
            searchNearButtons(&dummy, &next, &dummy, &dummy);
            select(next);
          }else{
            locationChg(getFocusedButton(), 0, 1);
          }
        }
        void moveLeft()
        {
          myButton *dummy, *next;
          if(editmode == false || getFocusedButton()->movable == false){
            searchNearButtons(&dummy, &dummy, &next, &dummy);
            select(next);
          }else{
            locationChg(getFocusedButton(), -1, 0);
          }
        }
        void moveRight()
        {
          myButton *dummy, *next;
          if(editmode == false || getFocusedButton()->movable == false){
            searchNearButtons(&dummy, &dummy, &dummy, &next);
            select(next);
          }else{
            if(getFocusedButton()->movable == true){
              locationChg(getFocusedButton(), 1,0);
            }else;
          }
        }

        void btnAction()
        {
          if(menuOpened == true)
          {
            return;
          }
          if(editmode == true)
          {
            myButton *editbtn = NULL;
            getFocusedButton(&editbtn);
            if(editbtn->movable == false){
              // ???????????????
              editbtn->movable = true;
              Serial.printf("[%s]movable TRUE\n", editbtn->name);
            }else{
              // ??????????????????
              editbtn->movable = false;
              Serial.printf("[%s]movable FALSE\n", editbtn->name);
            }
          }else{
            // ????????????????
            myButton *pushed = getFocusedButton();
            pushed->pressed = true;
            if(pushed->func != NULL){

            }else{
              Serial.println(" Action is not defined.");
            }
          }
        }

        void select(myButton *req)
        {
          myButton *next = NULL;
          myButton *prev = NULL;
          for(auto itr = btns.begin(); itr != btns.end(); ++itr)
          {
              if(*itr == req)
              {
                  next = *itr;
              }
              if((*itr)->focus == true)
              {
                  prev = *itr;
              }
          }
          if(next == prev){
            return;
          }
          Serial.printf(" >> %s >> %s\n", prev->name, next->name);
          if(next == NULL) { return; } 
          next->focus = true;

          if(prev == NULL) { return; }
          prev->focus = false;
        }

        void draw(LovyanGFX *base)
        {
          if(menuOpened == true)
          {
            /***********   MENU View         *****************/
            int ret = menu.update(&cambus);
            if(ret != 0){
              Serial.println("menu update!");
              cambus.pushSprite(base, 30, 30);
            }
          }else{
            /***********   Layout View         *****************/
            for(auto itr = btns.begin(); itr != btns.end();  ++itr)
            {
              (*itr)->clearBacklight(GRIDINTERVAL, &cambus);
            }
            for(auto itr = btns.begin(); itr != btns.end();  ++itr)
            {
              if((*itr)->focus == true){
                (*itr)->update();
                (*itr)->drawBacklight(GRIDINTERVAL, &cambus);
              }
            }

            if(editmode){
              for(int x = 0; x < cambus.width(); x+= GRIDINTERVAL){
                for(int y = 0; y < cambus.height(); y+= GRIDINTERVAL){
                  cambus.drawRect(x,y,GRIDINTERVAL,GRIDINTERVAL, TFT_YELLOW);
                }
              }
              /* ??????????????? */
              for(auto itr = btns.begin(); itr != btns.end(); ++itr)
              {
                if((*itr)->focus == true && (*itr)->movable == true){
                  (*itr)->draw(GRIDINTERVAL, &cambus, TFT_GOLD);
                }else{
                  (*itr)->draw(GRIDINTERVAL, &cambus);
                }
              }
            }else{          
              /* ??????????????? */
              for(auto itr = btns.begin(); itr != btns.end(); ++itr)
              {
                if((*itr)->pressed){
                  (*itr)->draw(GRIDINTERVAL, &cambus, TFT_DARKGREY);
                  (*itr)->pressed = false;
                }else{
                  (*itr)->draw(GRIDINTERVAL, &cambus);
                }
              }
            }
          }
          // ?????????????????????
          cambus.pushSprite(base, 30,30);
        }


        /********************* Setting Menu **********************/
        //void openSettingMenu(menuTree *target)
        //{
        //    /* MENU */
        //    menu.setMenuTree(*target);
        //    menu.begin();
        //}
//
        void openSettingMenu(LovyanGFX *base)
        {
        if(editmode == true && getFocusedButton()->movable == true &&
           menuOpened == false)
          {
            Serial.println(" << MENU OPEN >>");
            end();
            menu.init(base->width() * 0.3, base->height() * 0.4);
            menu.begin();                      /* ??????????????????????????? */
            menuOpened = true;
          }
        }
        
        void closeSettingMenu(LovyanGFX *base)
        {
            Serial.println(" << MENU EXIT >>");
            menu.end();                         /* ???????????????????????? */
            menuOpened = false;
            reset(base);
            draw(base);
        }

};