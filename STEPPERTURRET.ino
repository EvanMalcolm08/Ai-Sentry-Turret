// PROJECT   : Ai Sentry Turret
// PURPOSE   : The ai camera system that detects and sends data to arduino
// COURSE    : ICS3U-E1
// AUTHOR    : Evan M
// DATE      : 2025 04 05
// MCU       : 328P (Nano)
// STATUS    : Working
// REFERENCE : http://darcy.rsgc.on.ca/ACES/TEI3M/2425/ISPs.html 

#define DIR1 3    // Direction pin for Motor 1 (horizontal axis)
#define STEP1 4   // Step pin for Motor 1 (horizontal axis)
#define EN1 5     // Enable pin for Motor 1

#define DIR2 6    // Direction pin for Motor 2 (vertical axis)
#define STEP2 7   // Step pin for Motor 2 (vertical axis)
#define EN2 8     // Enable pin for Motor 2

const int deadzone = 20;          // The buffer area around the center where no movement occurs
const float step_scale = 0.1;     // Scale factor to convert pixel movement to stepper motor steps

void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);

  // Set motor control pins as OUTPUT
  pinMode(DIR1, OUTPUT);
  pinMode(STEP1, OUTPUT);
  pinMode(EN1, OUTPUT);

  pinMode(DIR2, OUTPUT);
  pinMode(STEP2, OUTPUT);
  pinMode(EN2, OUTPUT);

  // Enable motors by setting enable pins to LOW
  digitalWrite(EN1, LOW);
  digitalWrite(EN2, LOW);
  
  // Print a ready message for debugging
  Serial.println("Ready to track.");
}

void loop() {
  // Check if there is any data available from the serial port
  if (Serial.available()) {
    // Read the entire line of input
    String input = Serial.readStringUntil('\n');
    input.trim();  // Remove any leading or trailing whitespace characters

    // Declare variables to store the x and y displacement values
    int dx = 0;
    int dy = 0;

    // Parse the serial input (e.g., "X:30,Y:-40") and store values in dx and dy
    if (parseInput(input, dx, dy)) {
      // If the horizontal displacement is greater than the deadzone, move the horizontal motor
      if (abs(dx) > deadzone) {
        moveMotor(DIR1, STEP1, dx);  // Move the horizontal motor (left or right)
      }

      // If the vertical displacement is greater than the deadzone, move the vertical motor
      if (abs(dy) > deadzone) {
        moveMotor(DIR2, STEP2, dy);  // Move the vertical motor (up or down)
      }
    }
  }
}

// Parses the input string and extracts the X and Y displacement values
// Returns true if the parsing is successful
bool parseInput(String data, int &dx, int &dy) {
  // Find the position of "X:" and "Y:" in the input string
  int xIndex = data.indexOf("X:");
  int yIndex = data.indexOf("Y:");

  // If either "X:" or "Y:" is missing, return false (parsing error)
  if (xIndex == -1 || yIndex == -1) return false;

  // Extract the numerical values after "X:" and "Y:"
  String xVal = data.substring(xIndex + 2, data.indexOf(',', xIndex)); // Extract X value
  String yVal = data.substring(yIndex + 2); // Extract Y value

  // Convert the extracted values to integers and assign to dx and dy
  dx = xVal.toInt();
  dy = yVal.toInt();

  // Return true to indicate successful parsing
  return true;
}

// Moves the stepper motor based on the pixel displacement (dx or dy)
void moveMotor(int dirPin, int stepPin, int distance) {
  // Calculate the number of steps to move based on the pixel distance and the scale factor
  int steps = abs(distance) * step_scale;

  // Determine the direction of movement: forward or backward
  if (dirPin == DIR2) { 
    // Reverse direction for the vertical motor (DIR2) to make it move up if distance is negative
    digitalWrite(dirPin, distance < 0 ? HIGH : LOW);  
  } else {
    // For horizontal motor (DIR1), move right if distance is positive, left if negative
    digitalWrite(dirPin, distance > 0 ? HIGH : LOW);  
  }

  // Loop to move the motor the calculated number of steps
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);  // Step signal HIGH to move the motor
    delayMicroseconds(20);      // Delay to control step speed
    digitalWrite(stepPin, LOW);   // Step signal LOW to complete one step
    delayMicroseconds(20);      // Delay to control step speed
  }
}
