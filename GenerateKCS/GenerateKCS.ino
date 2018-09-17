//
// Generates a Kansas City Standard compatible audio output
// on the selected PWM pin.
//
// This is part of the KCSviewer repo at https://github.com/SmallRoomLabs/KCSviewer
// for the RetroChallenge 2018/09 competition
//
//

#define REFCLK (16000000.0/510.0)


#define MARK 1
#define MARK_CYCLES 8
#define MARK_FREQ 2400.0
#define MARK_INCR (4294967296.0*MARK_FREQ)/REFCLK

#define SPACE 0
#define SPACE_CYCLES 4
#define SPACE_FREQ 1200.0
#define SPACE_INCR (4294967296.0*SPACE_FREQ)/REFCLK

#define IDLE 65535

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

const uint8_t sine256[]  = {
  127, 130, 133, 136, 139, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173,
  176, 178, 181, 184, 187, 190, 192, 195, 198, 200, 203, 205, 208, 210, 212, 215,
  217, 219, 221, 223, 225, 227, 229, 231, 233, 234, 236, 238, 239, 240, 242, 243,
  244, 245, 247, 248, 249, 249, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254,
  254, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250, 249, 249, 248, 247, 245,
  244, 243, 242, 240, 239, 238, 236, 234, 233, 231, 229, 227, 225, 223, 221, 219,
  217, 215, 212, 210, 208, 205, 203, 200, 198, 195, 192, 190, 187, 184, 181, 178,
  176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 139, 136, 133, 130,
  128, 124, 121, 118, 115, 111, 108, 105, 102,  99,  96,  93,  90,  87,  84 , 81,
  78,  76,  73,  70,  67,  64,  62,  59,  56,  54,  51,  49,  46,  44,  42,  39,
  37,  35,  33,  31,  29,  27,  25,  23,  21,  20,  18,  16,  15,  14,  12,  11,
  10,   9,   7,   6,   5,   5,   4,   3,   2,   2,   1,   1,   1,   0,   0,   0,
  0,   0,   0,   0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,
  10,  11,  12,  14,  15,  16,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,
  37,  39,  42,  44,  46,  49,  51,  54,  56,  59,  62,  64,  67,  70,  73,  76,
  78,  81,  84,  87,  90,  93,  96,  99, 102, 105, 108, 111, 115, 118, 121, 124
};


//const uint8_t bits[]  = {0, 1, 0, 0, 0, 0, 0, 0, 0};  // Short for testing the Audio->Serial pcb

const uint8_t bits[]  = {           // For startbit detection
  0,  1, 0, 1, 0, 1, 0, 1, 0,   1,
  0,  0, 0, 0, 0, 0, 0, 0, 0,   1,
  0,  1, 1, 1, 1, 1, 1, 1, 1,   1,
  0,  1, 1, 1, 1, 1, 1, 1, 1,   1,1,1,1,1,
  0,  1, 1, 1, 0, 0, 1, 1, 1,   1
};

uint8_t sequencelength = sizeof(bits);

volatile uint16_t bitp = IDLE;

#define PWMPIN 3
#define BITPIN 2

void setup() {
  Serial.begin(115200);
  Serial.println("Kansas City Standard tape simulator");

  pinMode(PWMPIN, OUTPUT);     // PWM pin for OCR2B
  pinMode(BITPIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Timer2 Clock Prescaler to /1
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 PWM Mode set to Phase Correct PWM
  cbi (TCCR2A, COM2A0);  // Clear the Compare Match
  sbi (TCCR2A, COM2A1);

  cbi (TCCR2A, COM2B0);  // Clear the Compare Match
  sbi (TCCR2A, COM2B1);


  sbi (TCCR2A, WGM20);  // Mode 1, Phase correct PWM
  cbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);

  OCR2B = 128;

  // disable interrupts to avoid timing distortion
  cbi (TIMSK0, TOIE0);             // disable Timer0 aka delay()
  sbi (TIMSK2, TOIE2);             // enable Timer2 Interrupt
}


//
//
//
void Delay1ms() {
  for (uint16_t j = 0; j < 2500; j++) __asm__ __volatile__ ("nop\n\t");
}


//
//
//
void loop() {
  while (bitp != IDLE);
  digitalWrite(LED_BUILTIN, LOW);  
  for (uint16_t i = 0; i < 1000; i++) Delay1ms();
  bitp = 0;
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
}

//
//
//
ISR(TIMER2_OVF_vect) {
  static uint32_t accu = 0;    // phase accumulator
  static uint32_t  incr = MARK_INCR;    // accumulator increment value
  static uint8_t firstcycle = true;
  static uint8_t cyclecnt = 1;
  uint8_t isin;                   // index into sine table

  accu = accu + incr; // soft DDS, phase accu with 32 bits
  isin = accu >> 24;   // use upper 8 bits for phase accu as frequency information
  OCR2B = sine256[isin];;    // read from sine table and send vauleto PWM DAC

  if (isin > 127) {
    if (firstcycle) {
      firstcycle = false;
      cyclecnt--;
      if (cyclecnt == 0) {
        if ((bitp == IDLE)  || (bits[bitp++] == MARK)) {
          incr = MARK_INCR;
          cyclecnt = MARK_CYCLES;
          sbi(PORTD, BITPIN);
        } else {
          incr = SPACE_INCR;
          cyclecnt = SPACE_CYCLES;
          cbi(PORTD, BITPIN);
        }
        if (bitp >= sequencelength) bitp = IDLE;
      }
    }
  } else firstcycle = true;
}
