#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>

// 1. Create SoftwareSerial port on pins 2 (RX) and 3 (TX)
SoftwareSerial rs485(2, 3); 

// 2. Create the Modbus Slave object using our serial port.
// Because your HW-726 handles direction automatically, we DO NOT pass any DE/RE pins.
// The library defaults them to -1 (disabled) if you leave them out.
ModbusRTUSlave modbus(rs485);

#define LED_PIN LED_BUILTIN

// 3. Define the array that will act as our Holding Registers
// The library will automatically link Modbus registers to this array!
// Total 10 registers: 40001 = LED, 40002-40010 = general purpose
const uint8_t numHoldingRegisters = 10;
uint16_t holdingRegisters[numHoldingRegisters];

// RANDOM_REGISTER: -1 = disabled, >=0 = index of the holding register
// that the slave fills with a random value on every loop() iteration.
// Example: RANDOM_REGISTER=1 -> register 40002 will be random.
const int16_t RANDOM_REGISTER = -1;
static_assert(RANDOM_REGISTER < 0 || RANDOM_REGISTER < numHoldingRegisters,
              "RANDOM_REGISTER out of range (must be < numHoldingRegisters or -1)");

void setup() {
  // Start USB Serial for debugging on your PC
  Serial.begin(9600);
  Serial.println("Modbus Slave Starting...");

  // Initialize the LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Clear all registers to known defaults
  memset(holdingRegisters, 0, sizeof(holdingRegisters));

  // Link the Modbus Holding Registers to our C++ array
  modbus.configureHoldingRegisters(holdingRegisters, numHoldingRegisters);

  // Seed the PRNG from noise on A0 so the random register changes every cycle
  randomSeed(analogRead(A0));

  // Start the RS485 serial port at 9600 baud
  rs485.begin(9600); 

  // Initialize Modbus: (Slave ID, Baud Rate, Serial Config)
  // Slave ID = 1, Baud = 9600, Config = 8 data bits, No parity, 1 stop bit
  modbus.begin(1, 9600, SERIAL_8N1);
  
  Serial.println("Waiting for Master commands...");
  Serial.print("Holding registers: ");
  Serial.print(numHoldingRegisters);
  Serial.print(" (40001-400");
  Serial.print(numHoldingRegisters, DEC);
  Serial.println(")");
}

void loop() {
  // THIS IS CRITICAL: poll() must be called continuously to listen for the Master
  modbus.poll();

  // Random register: the slave fills this one with a fresh uint16 every cycle.
  // RANDOM_REGISTER = -1 disables this behavior (register stays as written by the Master).
  if (RANDOM_REGISTER >= 0) {
    holdingRegisters[RANDOM_REGISTER] = (uint16_t)random(0, 65536);
  }

  // Read Holding Register 40001 (index 0)
  // The library automatically updates this array in the background when the Master writes to it!
  uint16_t ledValue = holdingRegisters[0];

  // Control the LED based on the register value
  if (ledValue > 0) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}