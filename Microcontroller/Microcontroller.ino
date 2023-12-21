void setup() {
  Serial.begin(9600);
}

void loop() {
  String incomming_line = Serial.readStringUntil('\n');
  int first_operand_end = incomming_line.indexOf('\t');
  String first_operand = incomming_line.substring(0, first_operand_end);
  int operator_end = incomming_line.indexOf(0, first_operand_end);
  String operator_string = incomming_line.substring(first_operand_end, operator_end);
  String second_operatand = incomming_line.substring(operator_end, -1);
  Serial.println(incomming_line);
  Serial.println(first_operand);
  Serial.println(operator_string);
}
