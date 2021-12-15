/*  Librerías usadas */
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

/*  Entradas de la placa necesarias para la definición de la pantalla, el resto se encuentran implícitas */
#define TFT_DC 9
#define TFT_CS 10

/*  Definición de la pantalla */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
int width = 240;
int height = 320;

/*  Variable que representa la pantalla en la que se encuentra el programa  */
int pantalla;

/*  Variables relacionadas con el menú de selección */
int grosor;
bool paintMenu;
int menuOptions[2] = {28, 58};
int menuPosition;

/*  Definición de la entrada referente a los botones */
#define buttonNONE 0
#define buttonDOWN 5
#define buttonRIGHT 2
#define buttonA 6
#define buttonUP 3
#define buttonLEFT 4

/*  Definición del pin del piezoeléctrtrico */
#define buzzer 8

/*  Variables de estado */
int state_left = LOW;
int state_right = LOW;
int state_up = LOW;
int state_down = LOW;
int state_a = LOW;

/*  Variables referente a los colores */
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFD20

/*  Variables referentes a los sonidos */
#define DO 261.626 // Frecuencia en hercios de las notas musicales
//#define RE 293.665
#define MI 329.628
//#define FA 349.228
#define SOL 391.995
#define LASOS 465.000
#define LA 440.000
#define SI 493.883
#define DO2 523.251


/*  Canciones para las diversas pantallas */
//float canciónMenú[] = {SI,LA,MI,NONE,FA,SOL,DO2,NONE,DO,LA,SOL}
//float canciónBreakOut[] = {DO,LA,DO,LA,DO,LA,DO,NONE,NONE,SI,SOL,SI,SOL,SI,SOL,SI,NONE,NONE,MI2, RE2,SI}
//float canciónSnake[] = {SOL, NONE,NONE, SI, NONE,SI, NONE, SI,NONE, SOL, NONE, NONE, LA, NONE ,LA, NONE}
float gameOver[] = {DO2, SI, LASOS};
float winning[] = {SOL, SOL, SOL, DO2};

/*
 *  Variables y declaraciones necesarias para el juego de SNAKE
 */
uint16_t pixelColor;
uint8_t buttonPressed, currentDirection;
boolean collision, appleOn, displayEnd, specialOn, allowSpecial, showTitle = true, updateSelection;
int head, timer, snakeSize, score, pixelLocationXAxis, pixelLocationYAxis, x[300], y[300],
    appleX, appleY, yMultiplier, selection = 100, difficulty, specialX, specialY, specialTime;


/*
 * Variables y declaraciones necesarias para el juego BREAKOUT
 */
int positionX = 120;  //ball X position
int positionY = 240;  //ball Y position
int speedX = 6;
int speedY = -6;
int breakout_x = 95;         //paddle pos var
int breakout_y = 260;        //paddle pos var
int widthPaddle = 50;   
short collisions[4][4]= {{1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {1,1,1,1}};
boolean start = false;
boolean restart = false;


/*  Función setup */
void setup(void) {
  /* Configuración del puerto serie */
  Serial.begin(9600);
  
  /* Configuración inicial de la pantalla */
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(BLACK);
  
  /* Configuración de los botones como entrada  */
  pinMode(buttonLEFT, INPUT);
  pinMode(buttonRIGHT, INPUT);
  pinMode(buttonA, INPUT);
  pinMode(buttonDOWN, INPUT);
  pinMode(buttonUP, INPUT);

  /* Configuración del buzzer como una salida */
  pinMode(buzzer, OUTPUT);
  
  /*
   * Código de setup de la pantalla y el menú
   */
  pantalla = 1;
  paintMenu = true;
  grosor = 5;
  menuPosition = 0;
  

  /*
   * Código de setup del juego SNAKE
   */

   /*
  if (showTitle)
    showTitlScreen();*/
  tft.fillScreen(BLACK);
  collision = false;
  appleOn = false;
  specialOn = false;
  allowSpecial = false;
  displayEnd = false;
  pixelLocationXAxis = tft.width() / 2 - 1;
  pixelLocationYAxis = tft.height() / 2 - 1;
  pixelColor = WHITE;
  buttonPressed = buttonRIGHT;
  currentDirection = buttonRIGHT;
  specialTime = 0;
  score = 0;
  head = 0;
  snakeSize = 10;
  timer = 0;
  updateScore(score);
  updateSelection = true;

  menu();
}

/*-------------Funciones referentes a la lectura de los botones-------------*/
void checkButtons() {
  state_left = digitalRead(buttonLEFT);
  state_right = digitalRead(buttonRIGHT);
  state_a = digitalRead(buttonA);
  state_down = digitalRead(buttonDOWN);
  state_up = digitalRead(buttonUP);
  delay(100);
}

uint8_t readButton(void) {
  
  if (state_down) return buttonDOWN;
  if (state_right) return buttonRIGHT;
  if (state_a) return buttonA;
  if (state_up) return buttonUP;
  if (state_left) return buttonLEFT;
  else return buttonNONE;
}
/*-----------------------------------------------------------*/

/*-------------Funciones del juego BREAKOUT Parte 1-------------*/
void drawrect() {  
  unsigned int color[4] = {RED, ORANGE, YELLOW, GREEN};
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 4; j++){
      if(collisions[i][j] == 1)
        tft.fillRoundRect(j*60, (30 + i*11), 59, 10, 3, color[i]);
    }
  }
 }

void breakoutMenu(){
   //Imprimir el menú start
    delay(300);
    checkButtons();
    while(state_a == LOW){
      checkButtons();
      tft.fillScreen(BLACK);
      tft.setCursor(60,140);
      tft.setTextSize(4);
      tft.setTextColor(GREEN);
      tft.print("START");
    }
}

void breakout() {
 

  //LADRILLOS
  //Borrar los ladrillos

  boolean hasCollided = false; //variable para evitar que se rompan dos bloques a la vez si se pasa justo entre medias
  
  for(int i = 0; i < 4 && !hasCollided; i++){
    for(int j = 0; j < 4 && !hasCollided; j++){
      //Los bloques ya colisionados no se comprueban
      if(collisions[i][j] == 1){
       //Tenemos que comprobar si hay colision con el bloque
        if (positionX >= j * 60 && positionX <= (j + 1) * 60 && positionY <= (40 + i*11) ) { //Si se golpea la caja
            //Actualizamos el array de colisiones
            collisions[i][j] = 0;
            //Borramos la caja y rebotamos
            lilSoundBreakingBlock();
            tft.fillRoundRect(j*60, 30 + 11*i, 59, 10, 3, BLACK);
            speedY *= -1;
            hasCollided = true;
        }
      }
    }
  }

  if(checkVictory()){
    restart = true;
    victoryBreakout();
    return;
  }
    

  
  //Barra que se mueve y bola
  tft.fillRect(breakout_x, breakout_y, widthPaddle, 10, WHITE); // draw paddle
  tft.fillCircle(positionX, positionY, 4, BLACK); //undraw ball
  positionX = positionX + speedX;
  positionY = positionY + speedY;
  tft.fillCircle(positionX, positionY, 4, WHITE); //draw the ball

  if(hasCollided)
    drawrect(); //volvemos a dibujar los ladrillos que quedan por problemas del buffer

  //Colisiones
  if ((positionX + speedX) <= 0 || positionX + 2 > (width - speedX)) { //si llegamos a las paredes laterales, rebotamos
    lilSoundTouchingBar();
    speedX *= -1;
  }
  if (positionY - 4 <= 0) { //si llegamos a la parte de arriba de la pantalla, rebotamos hacia abajo
    lilSoundTouchingBar();
    speedY *= -1;
  }
  if ((positionY + 4 >= breakout_y) && (positionY + 4 <= breakout_y + 10) && (positionX >= breakout_x) && (positionX <= breakout_x + widthPaddle)) { //Comprobamos la colisión la barra, si está en la altura 260 y entre la pos y la anchura de la barra
    lilSoundTouchingBar();
    speedY *= -1;
  }

  //Final del juego si la bola está por debajo de la barra y no se ha golpeado
  if(positionY >= 300){
    restart = true;
    lilSoundGameOver();
    endBreakout();
    
  }

  checkButtons();

  if (state_left == HIGH) {
    tft.fillRect(breakout_x, breakout_y, widthPaddle, 10, BLACK);
    if (breakout_x < 0) { //if paddle is at end of display, dont move it
      breakout_x = breakout_x + 0;
    } else { //move the paddle
      breakout_x = breakout_x - 20;
    }

  }
  if (state_right == HIGH) {
    tft.fillRect(breakout_x, breakout_y, widthPaddle, 10, BLACK);
    if (breakout_x > (width - widthPaddle)) { //if paddle is at end of display, dont move it
      breakout_x = breakout_x - 0;
    } else {  //move the paddle
      breakout_x = breakout_x + 20;
    }

  }
}

void breakoutEndScreen(){

  checkButtons();
  uint8_t titleButton = readButton();

  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  
  if (titleButton == buttonUP){
    selection--;
    updateSelection = true;
  }
  if (titleButton == buttonDOWN){
    selection++;
    updateSelection = true;
  }

  if(updateSelection){
    if (selection % 2 == 0){ // easy mode
          tft.setCursor(30, 80);
          tft.fillRect(25, 78, 190, 26, BLUE);
          tft.print(">Restart");
          tft.setCursor(30, 110);
          tft.fillRect(25, 108, 190, 26, BLACK);
          tft.print(">Menu");
        } else if (selection % 2 == 1){ // normal mode
          tft.setCursor(30, 80);
          tft.fillRect(25, 78, 190, 26, BLACK);
          tft.print(">Restart");  
          tft.setCursor(30, 110);
          tft.fillRect(25, 108, 190, 26, BLUE);
          tft.print(">Menu");
        }
    updateSelection = false;
  }

  if(titleButton == buttonA){
    if(selection % 2 == 1){
      pantalla = 1;
      paintMenu = true;
      start = false;
      restart = false;
      return;
    }else{
      restartBreakout();
      restart = false;
      tft.fillScreen(BLACK);
      updateSelection = true;
      drawrect();
      return;      
    }

  }
  
  delay(100);
  breakoutEndScreen();
}

void endBreakout(){
  margen();
  tft.setCursor(25,30);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.print("GAME OVER");
  breakoutEndScreen();
  
}

boolean checkVictory(){
  boolean victory = true;
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 4; j++){
      if(collisions[i][j] == 1) //Si hay algún ladrillo aún sin colisionar, no hay victoria
        victory = false;
    }
  }
  return victory;
}
void victoryBreakout(){
  tft.fillScreen(WHITE);
  tft.setCursor(40,180);
  tft.setTextSize(4);
  tft.setTextColor(GREEN);
  tft.print("RESTART");
  tft.setCursor(40,80);
  tft.setTextSize(4);
  tft.setTextColor(BLACK);
  tft.print("VICTORY!");
  lilSoundWinning();
}
void restartBreakout(){
  positionX = 120;  //ball X position
  positionY = 240;  //ball Y position
  speedX = 6;
  speedY = -6;
  breakout_x = 95;         //paddle pos var
  breakout_y = 260;        //paddle pos var
  widthPaddle = 50;   
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      collisions[i][j]= 1;
    }
  }
}
/*-----------------------------------------------------------*/

/*-------------Función loop-------------*/
void loop() {
  switch(pantalla){
    case(1):
    {
      if(paintMenu){
        menu();
        paintMenu = false;
      }
      
      if(digitalRead(buttonDOWN) == HIGH){
        lilSoundMenuNavigation();
        paintMenu = true;
        delay(100);
      }

      if(digitalRead(buttonUP) == HIGH){
        lilSoundMenuNavigation();
        paintMenu = true;
        delay(100);
      }

      if(digitalRead(buttonA) == HIGH){
        lilSoundMenuNavigation();
        switch(menuPosition){
          case 0:
            showTitle = true;
            pantalla = 2;
            break;
          case 1:
            pantalla = 3;
            restart = false; 
            restartBreakout();
            break;
        }
      }
  }
      break;
      
    case(2):
    {
      if(showTitle){
        showTitlScreen();
      }
      checkButtons();
      uint8_t b = readButton();
      if (b != buttonNONE && b != buttonA)
        buttonPressed = b;
      if (!collision){
        appleLogic();
        checkIfAppleGot();
        specialApple();
        checkIfSpecialGot();
        specialTimer();
        updateSnakePosition(buttonPressed);
        if (timer >= snakeSize - 1)
          removeLastFromTail();
        else
          timer++;
      } else {
        if (displayEnd == true)
          displayEndingScreen();
          uint8_t buttonPressed = readButton();
          if (buttonPressed == buttonA){
            setup();
          }
      }
      delay(difficulty);
    }
      break;
      
    case(3):
    {
      //Solo se imprime esta pantalla una vez
      if(!start && !restart){
        breakoutMenu();
        checkButtons();
        //Si se pulsa el boton A
        if(state_a == HIGH){
          lilSoundMenuNavigation();
          start = true;
          tft.fillScreen(BLACK);
          drawrect();
          breakout();
        }
        delay(50);
      //esta pantalla se tiene que imprimir cada vez que vayamos a reiniciar el juego
      }else if(restart){
        checkButtons();
        //Si se pulsa el boton A reiniciamos el juego
        if(state_a == HIGH){
          //lilSoundMenuNavigation();
          restart = false;
          tft.fillScreen(BLACK);
          restartBreakout();
          drawrect();
          breakout();
        }
      //Se tiene que ejecutar el juego siempre que no estemos en una pantalla de seleccion
      }else{
        breakout();
      }
    }
    break;
  }
}
/*-------------------------------------*/

/*-------------Funciones del juego SNAKE Parte 2-------------*/
void showTitlScreen(){
  margen();
  tft.setTextSize(4);
  tft.setTextColor(WHITE);
  tft.setCursor(25, 30);
  tft.print("SNAKE");
  titleSelection();
}
  
void titleSelection(){
  tft.setTextSize(3);
  checkButtons();
  uint8_t titleButton = readButton();

  if (titleButton == buttonUP){
    selection--;
    updateSelection = true;
  }
  if (titleButton == buttonDOWN){
    selection++;
    updateSelection = true;
  }

  //Pinto en rojo el rectangulo que marca la opcion que voy a escoger
  if(updateSelection){
    if (selection % 3 == 0){ // easy mode
      lilSoundMenuNavigation();
      tft.setCursor(30, 80);
      tft.fillRect(25, 78, 190, 26, GREEN);
      tft.print(">Easy");
      tft.setCursor(30, 110);
      tft.fillRect(25, 108, 190, 26, BLACK);
      tft.print(">Medium");
      tft.setCursor(30, 140);
      tft.fillRect(25, 138, 190, 26, BLACK);
      tft.print(">Hard");
      difficulty = 200;
    } else if (selection % 3 == 1){ // normal mode
      lilSoundMenuNavigation();
      tft.setCursor(30, 80);
      tft.fillRect(25, 78, 190, 26, BLACK);
      tft.print(">Easy");  
      tft.setCursor(30, 110);
      tft.fillRect(25, 108, 190, 26, BLUE);
      tft.print(">Medium");
      tft.setCursor(30, 140);
      tft.fillRect(25, 138, 190, 26, BLACK);
      tft.print(">Hard");
      difficulty = 30;
    } else { // hard mode
      lilSoundMenuNavigation();
      tft.setCursor(30, 80);
      tft.fillRect(25, 78, 190, 26, BLACK);
      tft.print(">Easy");  
      tft.setCursor(30, 110);
      tft.fillRect(25, 108, 190, 26, BLACK);
      tft.print(">Medium");
      tft.setCursor(30, 140);
      tft.fillRect(25, 138, 190, 26, RED);
      tft.print(">Hard");
      difficulty = 1;
    }
    updateSelection = false;
  }
  
  if(titleButton == buttonA){
    lilSoundMenuNavigation();
    showTitle = false;
   //Imprimir el menú start
    delay(300);
    checkButtons();
    while(state_a==LOW){
    checkButtons();
    tft.fillScreen(BLACK);
    tft.setCursor(60,140);
    tft.setTextSize(4);
    tft.setTextColor(GREEN);
    tft.print("START");
    }
    tft.fillScreen(BLACK);
    drawBoard();
    return;
  }
  
  delay(100);
  titleSelection();
}

void drawBoard(){
  tft.drawRect(0, 9, tft.width(), tft.height() - 9, MAGENTA);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(3, 0);
  tft.print("Score:");
}

void updateScore(int score){
  tft.fillRect(38, 0, tft.width() - 75, 9, BLACK);
  tft.setTextSize(1);
  tft.setCursor(38, 0);
  tft.setTextColor(WHITE);
  tft.print(score);
}

void updateSnakePosition(uint8_t buttonPressed){

  if (buttonPressed == buttonUP){
    if(currentDirection != buttonDOWN){
      pixelLocationYAxis -= 3;
      currentDirection = buttonUP;
    } else
      pixelLocationYAxis += 3;
  }
  if (buttonPressed == buttonDOWN){
    if(currentDirection != buttonUP){
      pixelLocationYAxis += 3;
      currentDirection = buttonDOWN;
    } else
      pixelLocationYAxis -= 3;
  }  
  if (buttonPressed == buttonLEFT){
    if(currentDirection != buttonRIGHT){
      pixelLocationXAxis -= 3;
      currentDirection = buttonLEFT;
    } else
      pixelLocationXAxis += 3;
  }  
  if (buttonPressed == buttonRIGHT){
    if(currentDirection != buttonLEFT){
      pixelLocationXAxis += 3;
      currentDirection = buttonRIGHT;
    } else
      pixelLocationXAxis -= 3;
  }
    
  if (pixelLocationYAxis < 10)
    pixelLocationYAxis = 10;
  if (pixelLocationYAxis > tft.height() - 3)
    pixelLocationYAxis = tft.height() - 3;
  if (pixelLocationXAxis < 1)
    pixelLocationXAxis = 1;
  if (pixelLocationXAxis > tft.width() - 3)
    pixelLocationXAxis = tft.width() - 3;
    
  drawSnake(pixelLocationXAxis, pixelLocationYAxis, pixelColor);
  x[head] = pixelLocationXAxis;
  y[head] = pixelLocationYAxis;
  collisionCheck();
  head++;
  if (head >= 300){
    removeLastFromTail();
    resetArray();
    head = snakeSize + 1;
  }
}

void resetArray(){
  for(int j = 1; j < snakeSize; j++){
    x[snakeSize - j] = x[head - j];
    x[head - j] = 0;
    y[snakeSize - j] = y[head - j];
    y[head - j] = 0;
  }
  x[0] = 0;
  y[0] = 0;
}

void drawSnake(int pixelLocationXAxis, int pixelLocationYAxis, uint16_t pixelColor){

  tft.drawPixel(pixelLocationXAxis - 2, pixelLocationYAxis - 2, pixelColor);  //top left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis - 3, pixelColor);      //top
  tft.drawPixel(pixelLocationXAxis + 2, pixelLocationYAxis - 2, pixelColor);  //top right
  tft.drawPixel(pixelLocationXAxis - 2, pixelLocationYAxis, pixelColor);      //left
  tft.drawPixel(pixelLocationXAxis + 2, pixelLocationYAxis, pixelColor);      //right
  tft.drawPixel(pixelLocationXAxis - 2, pixelLocationYAxis + 2, pixelColor);  //bottom left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis + 3, pixelColor);      //bottom
  tft.drawPixel(pixelLocationXAxis + 2, pixelLocationYAxis +2, pixelColor);  //bottom right
  //Detalle
  tft.drawPixel(pixelLocationXAxis - 1.5, pixelLocationYAxis - 1.5, pixelColor);  //top left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis - 2.5, pixelColor);      //top
  tft.drawPixel(pixelLocationXAxis + 1.5, pixelLocationYAxis - 1.5, pixelColor);  //top right
  tft.drawPixel(pixelLocationXAxis - 1.5, pixelLocationYAxis, pixelColor);      //left
  tft.drawPixel(pixelLocationXAxis + 1.5, pixelLocationYAxis, pixelColor);      //right
  tft.drawPixel(pixelLocationXAxis - 1.5, pixelLocationYAxis + 1.5, pixelColor);  //bottom left
  tft.drawPixel(pixelLocationXAxis, pixelLocationYAxis + 2.5, pixelColor);      //bottom
  tft.drawPixel(pixelLocationXAxis + 1.5, pixelLocationYAxis +1.5, pixelColor);  //bottom right
}

void removeLastFromTail(){
  drawSnake(x[head - snakeSize], y[head - snakeSize], BLACK);
  x[head - snakeSize] = 0;
  y[head - snakeSize] = 0;
}

void collisionCheck(){
  if (x[head] < 2 || x[head] > tft.width() - 2 || y[head] <= 11 || y[head] >= tft.height()){
    pixelColor = RED;
    collision = true;
    drawSnake(pixelLocationXAxis, pixelLocationYAxis, RED);
    displayEnd = true;
  }
  
  for (int z = 0; z < head; z++) {
    if ((x[head] == x[z]) && (y[head] == y[z])) {
      pixelColor = RED;
      collision = true;
      drawSnake(pixelLocationXAxis, pixelLocationYAxis, RED);
      displayEnd = true;
    }
  }
}

void appleLogic(){
  if (!appleOn){
    appleX = random(2, tft.width() - 5);
    appleY = random(2, tft.height() - 5);
    drawApple(appleX, appleY, GREEN);
    appleOn = true;
  }
}

void checkIfAppleGot(){
  if ((x[head - 1] >= appleX-3) && (x[head - 1] <= appleX+3)){
    if ((y[head - 1] >= appleY-3) && (y[head - 1] <= appleY+3)){
      lilSoundEatingApple();
      drawApple(appleX, appleY, BLACK);
      appleOn = false;
      snakeSize += 2;
      score += 10;
      updateScore(score);
      allowSpecial = true;
   }
  } else
    drawApple(appleX, appleY, GREEN);
    
}

void specialApple(){
  if (!specialOn && (score % 100 == 0) && allowSpecial){
    specialX = random(2, tft.width() - 5);
    specialY = 10;
    yMultiplier = random(1, 49);
    if (specialX % 3 == 1)
      specialX--;
    else if (specialX % 3 == 2)
      specialX++;
    specialY += (3 * yMultiplier);
    drawApple(specialX, specialY, RED);
    specialTime = 0;
    specialOn = true;
    allowSpecial = false;
  }
}

void checkIfSpecialGot(){
  if (specialOn){
    if ((x[head - 1] == specialX)){
      if ((y[head - 1] == specialY)){
        specialSelection();
      }
    } else
      drawApple(specialX, specialY, RED);
  }
}

void specialSelection(){
  int removeSize = random(1, 5);
  int addedScore = random(1, 11) * 100;
  snakeSize -= removeSize;
  for (int i = 0; i < removeSize; i++){
    drawSnake(x[head - snakeSize - i], y[head - snakeSize - i], BLACK);
    x[head - snakeSize - i] = 0;
    y[head - snakeSize - i] = 0;
  }
  score += addedScore;
  updateScore(score);
  specialOn = false;
}

void specialTimer(){
  specialTime++;
  if (specialTime > 150){
    drawApple(specialX, specialY, BLACK);
    specialOn = false;
    allowSpecial == false;
    specialTime = 0;
  }
}

void drawApple(int pixelLocationXAxis, int pixelLocationYAxis, uint16_t pixelColor){
  tft.drawCircle(pixelLocationXAxis, pixelLocationYAxis,4,pixelColor);
  tft.fillCircle(pixelLocationXAxis, pixelLocationYAxis,4,pixelColor);
}

void displayEndingScreen(){
  tft.fillScreen(BLACK);
  tft.setCursor(100,250);
  tft.setTextSize(4);
  tft.setTextColor(BLUE);
  tft.print(score);
  tft.setCursor(55,190);
  tft.print("_____");
  tft.setCursor(55,180);
  tft.setTextSize(4);
  tft.print("SCORE");
  tft.setCursor(40,120);
  tft.setTextSize(4);
  tft.setTextColor(GREEN);
  tft.print("RESTART");
  tft.setCursor(15,60);
  tft.setTextSize(4);
  tft.setTextColor(RED);
  tft.print("GAME OVER");
  displayEnd = false;
  lilSoundGameOver();
}



/*-----------------------------------------------------------*/

/*-------------Funciones del menú-------------*/
void menu(){
  if(menuPosition == 0){
    menuPosition = 1;
  }else if(menuPosition == 1){
    menuPosition = 0;
  }

  updateSelection = true;
  
  margen();
  tft.setTextSize(3); 
  tft.fillRect(25, menuOptions[menuPosition], 195, 24, BLUE);
  tft.setCursor(25, 30);
  tft.print(">Snake");
  tft.setCursor(25, 60);
  tft.println(">Breakout");
  Serial.println(menuPosition);
}

void margen(){
  tft.fillScreen(BLACK);
  for(int i = 0; i < grosor; i++){
    tft.drawRect((10+i), (10+i), (220-(i*2)), (300-(i*2)), WHITE);
  }
}
/*--------------------------------------------*/

/*-----Funciones para reproduccir sonidos-----*/

void lilSoundMenuNavigation(){
  tone(buzzer, SI);
    delay(50);
  noTone(buzzer);
}

void lilSoundBreakingBlock(){
  tone(buzzer, MI);
    delay(50);
  noTone(buzzer);
}

void lilSoundTouchingBar(){
  tone(buzzer, SOL);
    delay(50);
  noTone(buzzer);
}

void lilSoundEatingApple(){
  tone(buzzer, LA);
  delay(50);
  noTone(buzzer);
  
}

void lilSoundGameOver(){
  for(int i = 0; i <=  2; i++){
    tone(buzzer, round(gameOver[i]));
    delay(500);
  }
  tone(buzzer, LA);
  delay(1000);
  noTone(buzzer);
}

void lilSoundWinning(){
  for(int i = 0; i <=  4; i++){
    tone(buzzer, round(winning[i]));
    delay(200);
  }
  noTone(buzzer);
}


/*--------------------------------------------*/
