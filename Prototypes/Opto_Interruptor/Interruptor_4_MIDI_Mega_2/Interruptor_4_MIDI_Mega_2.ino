/* 
* 2x2 matrix of Interruptors
* At this point most of this code is pulled directly from AccordionMega's code.
* Same as Prototype 2, but with MIDI printing instead
*/
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MIDI_CREATE_DEFAULT_INSTANCE();

char DIGITAL_COLUMNS[] = { 2,3,4,5 };

//Note: in this example we're only using two bits from each byte
int KeysStatus[] = {  
  B0000000,
  B0000000,
  B0000000,
  B0000000
};

//this is the value of all pins from the analog port (7 and 6 don't exist on UNO)
//B00000000
//B76543210
byte reg_values = 0;

void setup()
{
    MIDI.begin(1);
    //  Set MIDI baud rate:
    Serial.begin(115200);
    //Digital pins start turned off
    for (int i=0; i<sizeof(DIGITAL_COLUMNS);i++){ 
        pinMode(DIGITAL_COLUMNS[i],OUTPUT);
        digitalWrite(DIGITAL_COLUMNS[i], LOW);
    }

    //PortF as input 
    DDRF = B00000000;
    //Turn on pullup resistors
    PORTF = B11111111;

    delay(1000);
}

void loop()
{
  for (int i=0; i<sizeof(DIGITAL_COLUMNS);i++){ 
    //I wonder if we can replace this with direct port write for speed?
    digitalWrite(DIGITAL_COLUMNS[i], HIGH);
    //For some reason w/o a significant delay unpressed notes get played
    //Let's see if we can figure out a way to fix this...
    
    //If I bring this any lower nothing will sound - not sure why
    //(has something to do with the delay outside the digital write)
    delayMicroseconds(3000);//Orig val: 500 - TODO: play around with this number a bit
    
    //PINF = analog 0-7, PINK = analog 8-15
    reg_values = ~PINF;  
    
    digitalWrite(DIGITAL_COLUMNS[i], LOW);

    if (reg_values != KeysStatus[i]){
      if (reg_values > KeysStatus[i]){             
        check_key(reg_values ^ KeysStatus[i],i,true);  //sending modified bits only
      }
      else {
        check_key(reg_values ^ KeysStatus[i],i,false); //sending modified bits only
      }           
    }
    //I was able to bring the delay down this low by moving some of it here,
    //but it's still too high - definitely noticable when playing.
    delayMicroseconds(9000);
  }
}

void check_key(int reg, int group, boolean up){
   // saving 4 iterations, dividing byte by 2
   if (reg & 0xF0) {
     for(int i=0; i<4; i++){
       if ((reg >> 4+i) & 1){
         note_midi(group,i+4,up);
       }
     }
   }
   else if (reg & 0x0F) { 
    for(int i=0; i<4; i++){
       if ((reg >> i) & 1){
         note_midi(group,i,up);
       }
     }     
   }
}

//The right two most bits turn on because those pins don't exist on the Uno
const char right_notes_midi_numbers[][8] = {
  {53,55,57,59,60,62,64,65},//Added for test setup
  {55,59,58,57,56,55,54,53},
  {57,67,66,65,64,63,62,61},
  {59,75,74,73,72,71,70,69},
  {84,83,82,81,80,79,78,77},
  {92,91,90,89,88,87,86,85},
  {0,0,0,0,0,0,0,93}
};

byte midi_channel1 = 0;
byte midi_channel2 = 1;
char notes_to_play;

void note_midi(int group, int position, boolean on){
  int pitch;
  String str_warn;
  int midi_cmd = 1;
  char curr_register = 0;
  int midi_vel = 0;

  
  curr_register = 0;

  if(on) {
    //midi_cmd = midi_channel2 | 0x90;
    KeysStatus[group] |= (1 << position);  //setting bit value
    midi_vel = 127;
  }
  else if(~on) {
    //midi_cmd = midi_channel2 | 0x80;
    KeysStatus[group] &= ~(1 << position);  //setting bit value
    midi_vel = 0;
  }
  
  pitch = right_notes_midi_numbers[group][position];
  if (pitch){
    //For some reason both notes are getting played - why?
    if(on) {
      MIDI.sendNoteOn(pitch,midi_vel,midi_cmd);
    }
    else if(~on) {
      MIDI.sendNoteOff(pitch,midi_vel,midi_cmd);
    }
  }

}
