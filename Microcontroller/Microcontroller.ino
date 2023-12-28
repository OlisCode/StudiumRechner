#if not defined(__AVR_ATmega328P__)
#error "This programm has not been designed for this CPU. Please use a board with a compatible CPU"
#endif

void setup() {
  Serial.begin(9600);
}

void loop() {
  String incomming_line = Serial.readStringUntil('\n');
  int index_a = incomming_line.indexOf('A');
  int index_b = incomming_line.indexOf('B');
  int index_c = incomming_line.indexOf('C');
  int index_d = incomming_line.indexOf('D');
  // Only if every index character is found:
  if (index_a >= 0 && index_b >= 0 && index_c >= 0 && index_d >= 0) {
    String first_operand_str = incomming_line.substring(index_a + 1, index_b);
    String operator_str = incomming_line.substring(index_b + 1, index_c);
    String second_operand_str = incomming_line.substring(index_c + 1, index_d);
    if (is_full_digit(first_operand_str) && is_full_digit(second_operand_str) && is_operator(operator_str)) {
      float result = 0;
      float checkresult = 0;
      float first_operand = first_operand_str.toInt();
      float second_operand = second_operand_str.toInt();
      switch (byte(operator_str[0])) {
        case 0x2a:  // *
          result = first_operand * second_operand;
          checkresult = result / second_operand;
          Serial.println(result);
          break;
        case 0x2b:  // +
          result = first_operand + second_operand;
          Serial.println(result);
          break;
        case 0x2d:  // -
          result = first_operand - second_operand;
          Serial.println(result);
          break;
        case 0x2f:  // /
          if (second_operand == 0) {
            Serial.println("DIVBYZERO");
          } else {
            result = first_operand / second_operand;
            Serial.println(result);
          }
          break;
      }
    } else {
      Serial.println("FAIL");
    }
  }
}
bool is_full_digit(String tocheck) {
  bool toreturn = true;
  for (int i = 0; i < tocheck.length(); i++) {
    toreturn = toreturn & isDigit(tocheck[i]);
  }
  return toreturn;
}
bool is_operator(String tocheck) {
  if (tocheck.length() != 1) {
    return false;
  }
  switch (byte(tocheck[0])) {
    case 0x2a:  //*
      return true;
    case 0x2b:  //+
      return true;
    case 0x2d:  //-
      return true;
    case 0x2f:  ///
      return true;
    default:
      return false;
  }
}