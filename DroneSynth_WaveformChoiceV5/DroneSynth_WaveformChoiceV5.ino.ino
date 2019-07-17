/*
 * DRONE SYNTH!!!
 * V1: 2 voices with modulation with waveform swapping
 * V2: changes waveform swapping function to increase options, also improves debounce
 * V3: adds toggles to turn oscillators ON/OFF
 * V4: bit shifts output to accomodate HIFI output mode. More changes to come to take full advantage of the HIFI goodness.
 *     also changed debounce variables from int to unsigned long, and added a variable reset in code so they don't overflow.
 *     
 *IT'S DONE!!!!!!!!!!!!!!!
 *This will be the cheapo version. Moving on to upgraded version using a teensy3.2 board.
 *
 *V5: Psych! Wasn't done. Version 5 increases the size of the wavetables from 512 to 1024.
 *    It also swaps the handmade waveforms for newer, more interesting versions.
 */


//necessary Mozzi functions
#include <MozziGuts.h>
#include <Oscil.h>
#include <Smooth.h>
#include <AutoMap.h>
//wavetables (all same size for switching function)
#include <tables/sin1024_int8.h>
#include <tables/saw1024_int8.h>
#include <tables/square_no_alias1024_int8.h>
#include <tables/triangle1024_int8.h>
#include <tables/SawSin1024.h>
#include <tables/SquareSin1024.h>
#include <tables/whatamess1024.h>


// desired carrier frequency max and min, for AutoMap
const int MIN_CARRIER_FREQ = 0;
const int MAX_CARRIER_FREQ = 440;

// desired intensity max and min, for AutoMap, note they're inverted for reverse dynamics
const int MIN_INTENSITY = 700;
const int MAX_INTENSITY = 1;

// desired mod speed max and min, for AutoMap, note they're inverted for reverse dynamics
const int MIN_MOD_SPEED = 10000;
const int MAX_MOD_SPEED = 0;

//automap functions to more quickly map ADC inputs to desired values
AutoMap kMapCarrierFreq(0,1023,MIN_CARRIER_FREQ,MAX_CARRIER_FREQ);
AutoMap kMapIntensity(0,1023,MIN_INTENSITY,MAX_INTENSITY);
AutoMap kMapModSpeed(0,1023,MIN_MOD_SPEED,MAX_MOD_SPEED);

//PINS!
const byte CARRIER_PIN_1 = 0;
const byte MODULATOR_PIN_1 = 1;
const byte INTENSITY_PIN_1 = 2;
const byte CARRIER_PIN_2 = 3;
const byte MODULATOR_PIN_2 = 4;
const byte INTENSITY_PIN_2 = 5;
const byte WF_PIN_1A = 8; //aCarrier_1
const byte WF_PIN_1B = 7; //aCarrier_1
const byte WF_PIN_2A = 6; //aCarrier_2
const byte WF_PIN_2B = 5; //aCarrier_2
const byte WF_PIN_3A = 4; //aMod_1
const byte WF_PIN_4A = 3; //aMod_2
const byte ON_OFF_1 = 12; //voice 1 toggle
const byte ON_OFF_2 = 11; //voice 2 toggle

//Oscillator objects
Oscil<1024, AUDIO_RATE> aCarrier_1;
Oscil<1024, AUDIO_RATE> aModulator_1;
Oscil<1024, CONTROL_RATE> kIntensityMod_1;
Oscil<1024, AUDIO_RATE> aCarrier_2;
Oscil<1024, AUDIO_RATE> aModulator_2;
Oscil<1024, CONTROL_RATE> kIntensityMod_2;

//for toggle
boolean channel_1 = HIGH;  //0 = OFF, 1 = ON
boolean channel_2 = HIGH; //0 = OFF, 1 = ON
int channel_1_debounce = 0;
int channel_2_debounce = 0; 

// for controlling table swaps
boolean D1; 
boolean D2; 
boolean D3; 
boolean D4;
boolean D5; 
boolean D6;
int wfc_1 = 0;
int wfc_2 = 0;
int wfc_3 = 0;
int wfc_4 = 0;
int wfc_5 = 0;
int wfc_6 = 0;
int count1 = 0;
int count2 = 0;
int count3 = 0;
int count4 = 0;
int count5 = 0;
int count6 = 0;
unsigned long debounce1 = 0;
unsigned long debounce2 = 0;
unsigned long debounce3 = 0;
unsigned long debounce4 = 0;
unsigned long debounce5 = 0;
unsigned long debounce6 = 0;
int debounce_time = 13;


int mod_ratio = 5; // brightness (harmonics)
long fm_intensity_1; // carries control info from updateControl to updateAudio
long fm_intensity_2;


float smoothness = 0.95f;
Smooth <long> aSmoothIntensity(smoothness);


void setup() {
  
  startMozzi();
}


void updateControl(){
  //inc debounce values
  debounce1++;
  debounce2++;
  debounce3++;
  debounce4++;
  debounce5++;
  debounce6++;
  channel_1_debounce++;
  channel_2_debounce++;
  //read digital inputs to set wavetables
  D1 = digitalRead(WF_PIN_1A); //aCarrier_1
  D2 = digitalRead(WF_PIN_1B); //aCarrier_1
  D3 = digitalRead(WF_PIN_2A); //aCarrier_2
  D4 = digitalRead(WF_PIN_2B); //aCarrier_2
  D5 = digitalRead(WF_PIN_3A); //aMod_1
  D6 = digitalRead(WF_PIN_4A); //aMod_2
  //aCarrier_1
  if (D1 == HIGH and debounce1 > debounce_time) {
    count1++;
    wfc_1 = count1%7;
    debounce1 = 0;
  }
  switch (wfc_1) {
    case 0:
      aCarrier_1.setTable(SIN1024_DATA);
      break;
    case 1:
      aCarrier_1.setTable(SAW1024_DATA);
      break;
    case 2:
      aCarrier_1.setTable(SQUARE_NO_ALIAS1024_DATA);
      break;
    case 3:
      aCarrier_1.setTable(TRIANGLE1024_DATA);
      break;
    case 4:
      aCarrier_1.setTable(SAWSIN1024_DATA);
      break;
    case 5:
      aCarrier_1.setTable(SQUARESIN1024_DATA);
      break;
    case 6:
      aCarrier_1.setTable(WHATAMESS1024_DATA);
      break;
  }
  
  //aModulator_1
  if (D2 == HIGH and debounce2 > debounce_time) {
    count2++;
    wfc_2 = count2%7;
    debounce2 = 0;
  }
  switch (wfc_2) {
    case 0:
      aCarrier_1.setTable(SIN1024_DATA);
      break;
    case 1:
      aCarrier_1.setTable(SAW1024_DATA);
      break;
    case 2:
      aCarrier_1.setTable(SQUARE_NO_ALIAS1024_DATA);
      break;
    case 3:
      aCarrier_1.setTable(TRIANGLE1024_DATA);
      break;
    case 4:
      aCarrier_1.setTable(SAWSIN1024_DATA);
      break;
    case 5:
      aCarrier_1.setTable(SQUARESIN1024_DATA);
      break;
    case 6:
      aCarrier_1.setTable(WHATAMESS1024_DATA);
      break;
  }
  //kIntensityMod_1
  if (D3 == HIGH and debounce3 > debounce_time) {
    count3++;
    wfc_3 = count3 % 7;
    debounce3 = 0;
  }
  switch (wfc_3) {
    case 0:
      aCarrier_1.setTable(SIN1024_DATA);
      break;
    case 1:
      aCarrier_1.setTable(SAW1024_DATA);
      break;
    case 2:
      aCarrier_1.setTable(SQUARE_NO_ALIAS1024_DATA);
      break;
    case 3:
      aCarrier_1.setTable(TRIANGLE1024_DATA);
      break;
    case 4:
      aCarrier_1.setTable(SAWSIN1024_DATA);
      break;
    case 5:
      aCarrier_1.setTable(SQUARESIN1024_DATA);
      break;
    case 6:
      aCarrier_1.setTable(WHATAMESS1024_DATA);
      break;
  }
  //aCarrier_2
  if (D4 == HIGH and debounce4 > debounce_time) {
    count4++;
    wfc_4 = count4 % 7;
    debounce4 = 0;
  }
  switch (wfc_4) {
    case 0:
      aCarrier_1.setTable(SIN1024_DATA);
      break;
    case 1:
      aCarrier_1.setTable(SAW1024_DATA);
      break;
    case 2:
      aCarrier_1.setTable(SQUARE_NO_ALIAS1024_DATA);
      break;
    case 3:
      aCarrier_1.setTable(TRIANGLE1024_DATA);
      break;
    case 4:
      aCarrier_1.setTable(SAWSIN1024_DATA);
      break;
    case 5:
      aCarrier_1.setTable(SQUARESIN1024_DATA);
      break;
    case 6:
      aCarrier_1.setTable(WHATAMESS1024_DATA);
      break;
  }
  
  //aModulator_2
  if (D5 == HIGH and debounce5 > debounce_time) {
    count5++;
    wfc_5 = count5 % 7;
    debounce5 = 0;
  }
  switch (wfc_5) {
    case 0:
      aCarrier_1.setTable(SIN1024_DATA);
      break;
    case 1:
      aCarrier_1.setTable(SAW1024_DATA);
      break;
    case 2:
      aCarrier_1.setTable(SQUARE_NO_ALIAS1024_DATA);
      break;
    case 3:
      aCarrier_1.setTable(TRIANGLE1024_DATA);
      break;
    case 4:
      aCarrier_1.setTable(SAWSIN1024_DATA);
      break;
    case 5:
      aCarrier_1.setTable(SQUARESIN1024_DATA);
      break;
    case 6:
      aCarrier_1.setTable(WHATAMESS1024_DATA);
      break;
  }
  //kIntensity_2
  if (D6 == HIGH and debounce6 > debounce_time) {
    count6++;
    wfc_6 = count6 % 7;
    debounce6 = 0;
  }
  switch (wfc_6) {
    case 0:
      aCarrier_1.setTable(SIN1024_DATA);
      break;
    case 1:
      aCarrier_1.setTable(SAW1024_DATA);
      break;
    case 2:
      aCarrier_1.setTable(SQUARE_NO_ALIAS1024_DATA);
      break;
    case 3:
      aCarrier_1.setTable(TRIANGLE1024_DATA);
      break;
    case 4:
      aCarrier_1.setTable(SAWSIN1024_DATA);
      break;
    case 5:
      aCarrier_1.setTable(SQUARESIN1024_DATA);
      break;
    case 6:
      aCarrier_1.setTable(WHATAMESS1024_DATA);
      break;
  }

  if (digitalRead(ON_OFF_1) == HIGH and channel_1_debounce > debounce_time) {
    if (channel_1 == HIGH) {channel_1 = LOW;}
    else if (channel_1 == LOW) {channel_1 = HIGH;}
    channel_1_debounce = 0;
  }
  if (digitalRead(ON_OFF_2) == HIGH and channel_2_debounce > debounce_time) {
    if (channel_2 == HIGH) {channel_2 = LOW;}
    else if (channel_2 == LOW) {channel_2 = HIGH;}
    channel_2_debounce = 0;
  }

  int carrier_value_1, intensity_value_1, modulator_value_1; 
  int carrier_value_2, intensity_value_2, modulator_value_2;
  if (channel_1 == LOW) {
    carrier_value_1 = 0;
    intensity_value_1 = 1024;
    modulator_value_1 = 1024;
  }
  else {
    carrier_value_1 = mozziAnalogRead(CARRIER_PIN_1);
    intensity_value_1= mozziAnalogRead(INTENSITY_PIN_1);
    modulator_value_1 = mozziAnalogRead(MODULATOR_PIN_1);
  }
  int carrier_freq_1 = kMapCarrierFreq(carrier_value_1);
  int mod_freq_1 = carrier_freq_1 * mod_ratio;
  aCarrier_1.setFreq(carrier_freq_1); 
  aModulator_1.setFreq(mod_freq_1);
  int intensity_calibrated_1 = kMapIntensity(intensity_value_1);
  fm_intensity_1 = ((long)intensity_calibrated_1 * (kIntensityMod_1.next()+128))>>8;
  float mod_speed_1 = (float)kMapModSpeed(modulator_value_1)/1000;
  aCarrier_1.setFreq(carrier_freq_1); 
  aModulator_1.setFreq(mod_freq_1);
  kIntensityMod_1.setFreq(mod_speed_1);


  if (channel_2 == LOW) {
    carrier_value_2 = 0;
    intensity_value_2 = 1024;
    modulator_value_2 = 1024;
  }
  else {
    carrier_value_2 = mozziAnalogRead(CARRIER_PIN_2);
    intensity_value_2= mozziAnalogRead(INTENSITY_PIN_2);
    modulator_value_2 = mozziAnalogRead(MODULATOR_PIN_2);
  }
  int carrier_freq_2 = kMapCarrierFreq(carrier_value_2);
  int mod_freq_2 = carrier_freq_2 * mod_ratio;
  aCarrier_2.setFreq(carrier_freq_2); 
  aModulator_2.setFreq(mod_freq_2);
  int intensity_calibrated_2 = kMapIntensity(intensity_value_2);
  fm_intensity_2 = ((long)intensity_calibrated_2 * (kIntensityMod_2.next()+128))>>8;
  float mod_speed_2 = (float)kMapModSpeed(modulator_value_2)/1000;
  kIntensityMod_2.setFreq(mod_speed_2);

  /*
   * cycle debounce variables so they don't overflow -- 
   * only needs to happen VERY infrequently.
   */
   if (debounce1 > 4000000000) {debounce1 = debounce_time + 1;}
   if (debounce2 > 4000000000) {debounce2 = debounce_time + 1;}
   if (debounce3 > 4000000000) {debounce3 = debounce_time + 1;}
   if (debounce4 > 4000000000) {debounce4 = debounce_time + 1;}
   if (debounce5 > 4000000000) {debounce5 = debounce_time + 1;}
   if (debounce6 > 4000000000) {debounce6 = debounce_time + 1;}
}


int updateAudio(){
  long modulation_1 = aSmoothIntensity.next(fm_intensity_1) * aModulator_1.next();
  long modulation_2 = aSmoothIntensity.next(fm_intensity_2) * aModulator_2.next();
  return (aCarrier_1.phMod(modulation_1) + aCarrier_2.phMod(modulation_2)) << 5;  //V4 CHANGE FOR HIFI MODE
}


void loop() {
  audioHook();
}
