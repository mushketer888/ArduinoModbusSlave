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
const uint8_t numHoldingRegisters = 1;
uint16_t holdingRegisters[numHoldingRegisters];

void setup() {
  // Start USB Serial for debugging on your PC
  Serial.begin(9600);
  Serial.println("Modbus Slave Starting...");

  // Initialize the LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Link the Modbus Holding Registers to our C++ array
  modbus.configureHoldingRegisters(holdingRegisters, numHoldingRegisters);

  // Start the RS485 serial port at 9600 baud
  rs485.begin(9600); 

  // Initialize Modbus: (Slave ID, Baud Rate, Serial Config)
  // Slave ID = 1, Baud = 9600, Config = 8 data bits, No parity, 1 stop bit
  modbus.begin(1, 9600, SERIAL_8N1);
  
  Serial.println("Waiting for Master commands...");
}

void loop() {
  // THIS IS CRITICAL: poll() must be called continuously to listen for the Master
  modbus.poll();

  // Read Holding Register 0 (which is 40001 in Modbus terminology)
  // The library automatically updates this array in the background when the Master writes to it!
  uint16_t ledValue = holdingRegisters[0];

  // Control the LED based on the register value
  if (ledValue > 0) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}