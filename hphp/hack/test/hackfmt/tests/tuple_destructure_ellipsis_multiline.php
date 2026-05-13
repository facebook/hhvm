<?hh

function f((int, int, int, int, int) $t): void {
  tuple($long_variable_name_one, $long_variable_name_two, $long_variable_name_three, ...) = $t;
}
