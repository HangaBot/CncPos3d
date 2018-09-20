/*
 * Author: Alfonso Breglia
 * Release Date: 19/10/2018
*/

// TODO: modificare tutte le funzioni di movimento in modo da
// muover la macchina in una posizione assoluta

// arduino pin <--> pins on ULN2003 board:
// 8           <--> IN1
// 9           <--> IN2
// 10          <--> IN3
// 11          <--> IN4

  /* Note: CheapStepper library assumes you are powering your 28BYJ-48 stepper
   * using an external 5V power supply (>100mA) for RPM calculations
   * -- don't try to power the stepper directly from the Arduino
   * 
   * accepted RPM range: 6RPM (may overheat) - 24RPM (may skip)
   * ideal range: 10RPM (safe, high torque) - 22RPM (fast, low torque)
   */

#include <Servo.h>
#include <CheapStepper.h>

//Pin configuration
#define YSERVO_PIN  3
#define ZSERVO_PIN  5
#define FINECORSA1_PIN 6
#define FINECORSA2_PIN 7 

#define  IN1_PIN 8
#define  IN2_PIN 9
#define  IN3_PIN 10
#define  IN4_PIN 11


//Framer
#define MAX_MSG_LEN 16
#define FRAMER_IDLE  0
#define FRAMER_WAIT_ENDLINE 1

#define NO_MSG  0
#define LEN_ERR -1
#define NEW_MSG 1


//STEPPER 
#define MOVE_CW  true
#define MOVE_CCW false

struct contextt_t{
  //hidden context
  uint32_t  framer_context;
  char      msg_buffer[MAX_MSG_LEN];
  uint32_t  msg_buff_pos;
  Servo yservo;  // servo che controlla l'asse y
  Servo zservo;  // servo che controlla l'asse z 
  CheapStepper xstepper;

  int32_t xsteps;
  int32_t ysteps;
  int32_t zsteps;
  
}context; 

// FRAMER
void init_framer(){
  context.framer_context = FRAMER_IDLE;
  context.msg_buff_pos = 0;
  for(int i =0; i< MAX_MSG_LEN; i++){
    context.msg_buffer[i] = '\0';
  }
}

// do framer just analyze one character at time
int do_framer(){
  int res = NO_MSG;
  
  if (Serial.available()>0){
    char c = Serial.read();
    if(context.framer_context == FRAMER_IDLE){
        if(c == '!'){
          context.msg_buff_pos = 0;
          context.framer_context = FRAMER_WAIT_ENDLINE;
        }
    }else if (context.framer_context == FRAMER_WAIT_ENDLINE){
      if(context.msg_buff_pos <MAX_MSG_LEN-1){ // we need the space for the null terminatore
        //append the new character to the end of the buffer 
        context.msg_buffer[context.msg_buff_pos] = c; 
        context.msg_buff_pos++;
        
        if(c == '\n'){
            context.msg_buffer[context.msg_buff_pos] = '\0';  //add  the null-terminator

            context.framer_context = FRAMER_IDLE;
            context.msg_buff_pos = 0;
            res = NEW_MSG;
        }
      }else{
          context.framer_context = FRAMER_IDLE;
          context.msg_buff_pos = 0;
          res = LEN_ERR;
      }
    }else{// wrong context should never happens     
      context.framer_context = FRAMER_IDLE;
      context.msg_buff_pos = 0;
      res = LEN_ERR;
      
    }
  }
  return res;
}
// End FRAMER 


int safe_strcmp(const char *str1, const char *str2, int len){
  int j = 0;
  for(j = 0;   j < len ; j++){
    if (str2[j] == '*') 
      continue;
    if(str1[j] != str2[j])
      return 1;    
  }
  return 0;
}

uint32_t hexstr2uint(const char *str, int num_digit){ 
  
  uint32_t val = 0;
  for (int i = 0; i < num_digit; i++){
      int d= 0;
      if (str[i] <= 'F' && str[i] >= 'A'){
         d = str[i] - 'A' + 10; 
      }else if(str[i] <= 'f' && str[i] >= 'a'){
         d = str[i] - 'a' + 10;        
      }else if (str[i] <= '9' && str[i] >= '0'){
        d = str[i] - '0' ;        
      }
      
      val+= (d&0xF) << (4*(num_digit-i-1));
  }
  
  return val;
}


//interpret the command in the framer_buffer
void do_cmd(){
   
   if(safe_strcmp(context.msg_buffer, "X****\n", 6) == 0){
      uint16_t val =     hexstr2uint(context.msg_buffer+1,4);
      int16_t xsteps =  *((int16_t*)(&val));
      move_xaxis(xsteps);
      Serial.write(":OK\n");
      
   }else if(safe_strcmp(context.msg_buffer, "Y****\n", 6) == 0){  
      uint16_t val =hexstr2uint(context.msg_buffer+1,4);
      int16_t ysteps =  *((int16_t*)(&val));
      move_yaxis(ysteps);      
      Serial.write(":OK\n");
      
   }else if(safe_strcmp(context.msg_buffer, "Z****\n", 6) == 0){ 
      uint16_t val = hexstr2uint(context.msg_buffer+1,4);
      int16_t zsteps =  *((int16_t*)(&val));
      move_zaxis(zsteps);       
      Serial.write(":OK\n");
      
   }else {
      Serial.write(":ERR\n");
   }
   
   
}

void init_motor(){

  context.xsteps = 0;
  context.ysteps = 0;
  context.zsteps = 0;
  
  
  context.yservo.attach(YSERVO_PIN);
  context.zservo.attach(ZSERVO_PIN);


  pinMode(FINECORSA1_PIN, INPUT);
  pinMode(FINECORSA2_PIN, INPUT);
  context.xstepper.setRpm(10); 
  Serial.print(context.xstepper.getRpm()); // get the RPM of the stepper
  Serial.print(" rpm = delay of ");
  Serial.print(context.xstepper.getDelay()); // get delay between steps for set RPM
  Serial.print(" microseconds between steps");
  Serial.println();
}


int move_yaxis(int32_t steps){


    
    context.ysteps = steps;
    context.yservo.write(context.ysteps+90); 

}

int move_zaxis(int32_t steps){
    context.zsteps = steps;
    context.zservo.write(context.zsteps+90);
}

int move_xaxis(int32_t steps){

  int finecorsa_pin;
  int dir;
  int rot_sense;
  if(steps > 0){
    finecorsa_pin = FINECORSA2_PIN;
    dir           = 1;
    rot_sense           = MOVE_CCW;
  }else{
    finecorsa_pin = FINECORSA1_PIN;
    rot_sense           = MOVE_CW;  
    dir           = -1;
    steps             = -steps;  
  }
  for(int32_t i =0; i< steps; i++){
      if (digitalRead(finecorsa_pin) == 1){
        context.zsteps += dir;
        context.xstepper.step(rot_sense);
      }else{
        return 1; // trovato fine corsa
      }   
  }
  return 0;
}

void setup() {

  init_motor();
  init_framer();
  Serial.begin(9600);
  Serial.println("CnCPos3d");
  Serial.println(__DATE__ " " __TIME__ );
}

void loop() {
    int res = 0;
  
  res = do_framer();
  
  if (res == NEW_MSG){    
    do_cmd();    
  }else if (res == LEN_ERR){
    Serial.write(":ERR\n");
  } 

}
