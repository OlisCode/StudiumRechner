#if not defined(__AVR_ATmega328P__)
#error "This programm has not been designed for this CPU. Please use a board with a compatible CPU"
#endif

void setup() {
  Serial.begin(115200);
}

void loop() {
  //String incomming_line = readline();
  String incomming_line = Serial.readStringUntil('\n');
  int index_ab = incomming_line.indexOf("AB"); // If the fist 2 Characters are BB the package should not be answered
  int index_c = incomming_line.indexOf('C');
  int index_d = incomming_line.indexOf('D');
  int index_e = incomming_line.indexOf('E');
  int index_y = incomming_line.indexOf('Y');
  int index_z = incomming_line.indexOf('Z');
  // Only if every index character is found:
  if (index_ab >= 0 && index_c >= 0 && index_d >= 0 && index_e >= 0 && index_y >= 0 && index_z >= 0) {
    String checksum_from_host_str = incomming_line.substring(index_y + 1, index_z);
    uint8_t checksum_calculated = calculate_checksum(incomming_line.substring(index_ab, index_y+1));
    uint8_t checksum_from_host_uint8 = checksum_from_host_str.toInt();
    if (checksum_from_host_uint8 == checksum_calculated) {
      String first_operand_str = incomming_line.substring(index_ab + 2, index_c);
      String operator_str = incomming_line.substring(index_c + 1, index_d);
      String second_operand_str = incomming_line.substring(index_d + 1, index_e);
      if (is_full_digit(first_operand_str) && is_full_digit(second_operand_str)) {
        float result = 0;
        float checkresult = 0;
        float first_operand = first_operand_str.toInt();
        float second_operand = second_operand_str.toInt();
        switch (byte(operator_str[0])) {
          case 0x2a:  // *
            result = first_operand * second_operand;
            checkresult = result / second_operand;
            Serial.print(result);
            break;
          case 0x2b:  // +
            result = first_operand + second_operand;
            Serial.print(result);
            break;
          case 0x2d:  // -
            result = first_operand - second_operand;
            Serial.print(result);
            break;
          case 0x2f:  // /
            if (second_operand == 0) {
              Serial.print("DIVBYZERO");
            } else {
              result = first_operand / second_operand;
              Serial.print(result);
            }
            break;
          default:
            Serial.print("NOOP");
            break;
        }
      } else {
        Serial.print("FAIL");
      }
      Serial.print("\n");  //not using println because it also sends \r which is not desired
    } else {
      Serial.print("CHKSUM");
      Serial.print("\n");
    }
  }
}

String readline() {
  uint32_t starttime = millis();
  String toreturn = "";
  bool break_loop = false;
  while (!break_loop) {
    if (millis() > starttime + 1000)  //TODO theoretical error at overflow
    {
      toreturn = "";
      break_loop=true;
    }
    int in_byte = Serial.read();
    if (in_byte != -1) {
      Serial.println(String(char(in_byte)));
      toreturn=toreturn+String(char(in_byte));
      if (in_byte == int('\n')) {
        break_loop=true;
      }
    }
  }
}

uint8_t calculate_checksum(String message) {
  //TODO quint64 in theory limits string size/length
  uint8_t toreturn = 0;
  for (int i = 0; i < message.length(); i++) {
    toreturn += byte(message[i]);
  }
  return toreturn;
}

bool is_full_digit(String tocheck) {
  bool toreturn = true;
  for (int i = 0; i < tocheck.length(); i++) {
    toreturn = toreturn & isDigit(tocheck[i]);
  }
  return toreturn;
}