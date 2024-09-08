#include <SoftwareSerial.h>

SoftwareSerial SIM900(0, 1); // pin 0-TXD; pin 1-RXD
String inputString = ""; // String to hold user input

void setup() {
  Serial.begin(9600); // Start Serial Monitor at 9600 baud
  SIM900.begin(57600); // Start SIM900 at 57600 baud
  delay(20000); // Allow time for SIM900 to initialize

  // Set SMS mode to text
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // Configure to show SMS upon receipt
  SIM900.print("AT+CNMI=1,2,0,0,0\r");
  delay(100);

  Serial.println("SIM900 is ready.");
  Serial.println("Use 'send +<phone number> <message>' to send an SMS.");
  Serial.println("Use 'call +<phone number>' to make a call.");
  Serial.println("Use 'Answer' to answer an incoming call, and 'Reject' to reject.");
}

void loop() {
  // Continuously check for received SMS messages
  if (SIM900.available() > 0) {
    while (SIM900.available()) {
      char inchar = SIM900.read();
      Serial.print(inchar); // Debugging output
    }
  }

  // Continuously check for input from the serial console
  if (Serial.available() > 0) {
    char inchar = Serial.read(); // Read user input from Serial Monitor
    if (inchar == '\n' || inchar == '\r') {
      processCommand(inputString); // Process command when user hits enter
      inputString = ""; // Clear the input string
    } else {
      inputString += inchar; // Add incoming characters to the string
    }
  }
}

// Process the user's command entered via the Serial Monitor
void processCommand(String command) {
  command.trim(); // Remove any leading/trailing whitespace

  // Command to send an SMS
  if (command.startsWith("send ")) {
    String phoneNumber = extractPhoneNumber(command, 5);
    String message = extractMessage(command);

    if (phoneNumber != "" && message != "") {
      sendSMS(phoneNumber, message);
    } else {
      Serial.println("Invalid format! Use: send +<phone number> '<message>'");
    }
  }

  // Command to make a call
  else if (command.startsWith("call ")) {
    String phoneNumber = extractPhoneNumber(command, 5);
    phoneNumber.trim(); // Remove any leading/trailing spaces from the phone number
    if (phoneNumber != "") {
      makeCall(phoneNumber);
    } else {
      Serial.println("Invalid format! Use: call +<phone number>");
    }
  }

  // Command to answer a call
  else if (command == "Answer") {
    answerCall();
  }

  // Command to reject a call
  else if (command == "Reject") {
    rejectCall();
  }

  // Unknown command
  else {
    Serial.println("Unknown command. Please use 'send', 'call', 'Answer', or 'Reject'.");
  }
}

// Function to extract phone number from the command
String extractPhoneNumber(String command, int offset) {
  String phoneNumber = command.substring(offset); // Extract phone number till the end
  phoneNumber.trim(); // Trim any extra spaces
  return phoneNumber;
}

// Function to extract the message from the command
String extractMessage(String command) {
  int start = command.indexOf('\'');
  int end = command.lastIndexOf('\'');
  if (start != -1 && end != -1 && end > start) {
    return command.substring(start + 1, end); // Extract text inside the quotes
  }
  return "";
}

// Function to send an SMS
void sendSMS(String phoneNumber, String message) {
  Serial.println("Sending SMS to: " + phoneNumber); // Debugging output

  // Send AT+CMGS command with phone number
  SIM900.print("AT+CMGS=\"");
  SIM900.print(phoneNumber);
  SIM900.println("\"");
  delay(100);

  // Wait for the '>' prompt indicating readiness to receive the message
  unsigned long startTime = millis();
  String response = "";
  while (millis() - startTime < 5000) { // 5 seconds timeout
    if (SIM900.available()) {
      char c = SIM900.read();
      response += c;
      Serial.print(c); // Debugging output
      if (response.indexOf('>') != -1) { // Check if prompt '>' received
        SIM900.println(message); // Send the message
        delay(100); // Wait a bit before sending CTRL+Z
        SIM900.write(26); // ASCII code for CTRL+Z to send the message
        Serial.println("SMS Sent to " + phoneNumber + ": " + message);
        return;
      }
    }
  }
  
  Serial.println("Failed to send SMS. No prompt received.");
}

// Function to make a call
void makeCall(String phoneNumber) {
  Serial.println("Making call to: " + phoneNumber); // Debugging output
  SIM900.print("ATD");
  SIM900.print(phoneNumber);
  SIM900.println(";");
  Serial.println("Calling " + phoneNumber + "...");
}

// Function to answer an incoming call
void answerCall() {
  Serial.println("Answering call."); // Debugging output
  SIM900.println("ATA"); // Answer the call
  Serial.println("Call answered.");
}

// Function to reject an incoming call
void rejectCall() {
  Serial.println("Rejecting call."); // Debugging output
  SIM900.println("ATH"); // Hang up (reject) the call
  Serial.println("Call rejected.");
}
