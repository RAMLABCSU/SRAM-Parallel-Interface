#include<Arduino.h>

// Define serial port used
#define SERIAL_PORT SerialUSB

#define MAX_ADDR_LINES 32
#define MAX_DATA_LINES 32

bool label = true;

// Address and word size types, 
typedef uint32_t address_t;
typedef uint32_t word_t;

// Number of address and data lines
int numAddrLines = 18;
int numDataLines = 16;

// Set the SRAM size in number of words
int sizeSRAM = 262144;

int addrLines[MAX_ADDR_LINES];
int dataLines[MAX_DATA_LINES];

// Control lines:
// IDT71V416 pins
int chipEnableLine = 7;
int outputEnableLine = 47;
int writeEnableLine = 8;
int lowerByteEnable = 45;
int upperByteEnable = 46;
int ONOFF = 53;


//***// Functions begin here//***//

// Set up IDT71V416-specific pin mappings and general configuration
void configure_IDT71V416() {

  SERIAL_PORT.println("Configuring pins for IDT71V416...");

  // Configure pins for IDT71V416 chips according to pin numbers on TSOP 44 package

  // Address lines on SRAM pins 1-5, 18-22, 23-27, 42-44
  numAddrLines = 18;
  //addr line pin mapping
  int addrLinesMR[] = {2,  3,  4,  5,  6,  9,  10, 11, 12,
                        13, 32, 33, 34, 35, 36, 48, 49, 50};

  // Apply the address line mapping
  for (int i = 0; i < numAddrLines; i++) {
    addrLines[i] = addrLinesMR[i];
  }

  // I/O lines on SRAM pins 7-10, 13-16, 29-32, 35-38 are configured on each I/O
  // operation
  numDataLines = 16;

  // data line pin mapping
  int dataLinesMR[] = {22, 23, 24, 25, 26, 27, 28, 29,
                        37, 38, 39, 40, 41, 42, 43, 44};

  // Apply the data line mapping
  for (int i = 0; i < numDataLines; i++) {
    dataLines[i] = dataLinesMR[i];
  }


  //// General pin configuration////

  // Set address line settings
  for (int i = 0; i < numAddrLines; i++) {
    pinMode(addrLines[i], OUTPUT);
  }

  // Set control line settings

  pinMode(ONOFF, OUTPUT);
  digitalWrite(ONOFF, LOW);

  pinMode(chipEnableLine, OUTPUT);
  digitalWrite(chipEnableLine, LOW); // Set to enable chip

  pinMode(outputEnableLine, OUTPUT);
  digitalWrite(outputEnableLine, LOW); // Set to enable chip output

  pinMode(writeEnableLine, OUTPUT);
  digitalWrite(writeEnableLine, HIGH); // Set to disable writing at start

  pinMode(lowerByteEnable, OUTPUT);
  digitalWrite(lowerByteEnable, LOW); // Set to enable reading/writing lower byte

  pinMode(upperByteEnable, OUTPUT);
  digitalWrite(upperByteEnable, LOW); // Set to enable reading/writing upper byte

}

// Set the address lines to the provided address
void setAddress(address_t address) {

  // Set address line values
  for (int i = 0; i < numAddrLines; i++) {
    digitalWrite(addrLines[i], address & (0x01 << i));
  }
}

// Reads a word from the SRAM at a defined address
word_t readWord(address_t address) {

  word_t data = 0x0000;

  // Unset write enable to begin reading
  digitalWrite(writeEnableLine, HIGH);

  // Set I/O line settings
  for (int i = 0; i < numDataLines; i++) {
    pinMode(dataLines[i], INPUT);
  }

  // Reset the I/O lines for testing
  for (int i = 0; i < numDataLines; i++) {
    digitalWrite(dataLines[i], 0);
  }

  // Set the address to read from and enable chip output
  setAddress(address);
  digitalWrite(outputEnableLine, LOW);

  // Delay 1 us to let data settle
  delayMicroseconds(1);

  // Read data from MRAM
  for (int i = 0; i < numDataLines; i++) {
    data |= (digitalRead(dataLines[i]) << i);
  }

  // Disable chip output
  digitalWrite(outputEnableLine, HIGH);

  return data;
}

// Writes a word to the MRAM at the provided address
void writeWord(address_t address, word_t data) {
  // Unset output enable
  digitalWrite(outputEnableLine, HIGH);

  // Set I/O line settings
  for (int i = 0; i < numDataLines; i++) {
    pinMode(dataLines[i], OUTPUT);
  }

  // Set data to write
  for (int i = 0; i < numDataLines; i++) {
    digitalWrite(dataLines[i], data & (0x01 << i));
  }

  // Set the address to write to
  setAddress(address);

  // Set the write enable
  delayMicroseconds(1);
  digitalWrite(writeEnableLine, LOW);

  // Delay 1 us to let data settle
  delayMicroseconds(1);

  // Unset the write enable, set output enable
  digitalWrite(writeEnableLine, HIGH);
  delayMicroseconds(1);
}


//**// Functions end here //**//



void setup() {
  // Configure serial interface
  SERIAL_PORT.begin(115200);
  SERIAL_PORT.setTimeout(10);

  while(!SERIAL_PORT);

  SERIAL_PORT.println("\n\rInitializing Serial Interface...\n\r");

  // Prompt for chip configuration
  configure_IDT71V416();

}


void loop () {

  word_t data_1 = 0xffc5; // data that we want to write
  
  while(label){

    for (address_t  j = 0; j < sizeSRAM; j++) {

      writeWord(j, data_1);
      
    }

    SERIAL_PORT.print("Done Writing to all words !!");

    label=false;
  }
      
}
