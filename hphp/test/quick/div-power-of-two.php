<?hh

// Prevent constant folding
function get_value(int $val) :int{
  return HH\Lib\PseudoRandom\int($val, $val);
}

<<__EntryPoint>>
function main(): void {
  // Divide by 8
  var_dump(get_value(64) / 8);
  var_dump(get_value(32) / 8);
  var_dump(get_value(16) / 8);
  var_dump(get_value(8) / 8);
  var_dump(get_value(0) / 8);
  var_dump(get_value(-8) / 8);
  var_dump(get_value(-16) / 8);

  // Divide by negative 8
  var_dump(get_value(64) / -8);
  var_dump(get_value(32) / -8);
  var_dump(get_value(16) / -8);
  var_dump(get_value(8) / -8);
  var_dump(get_value(0) / -8);
  var_dump(get_value(-8) / -8);
  var_dump(get_value(-16) / -8);

  // Divide by 2
  var_dump(get_value(16) / 2);
  var_dump(get_value(10) / 2);
  var_dump(get_value(8) / 2);
  var_dump(get_value(4) / 2);
  var_dump(get_value(2) / 2);
  var_dump(get_value(0) / 2);
  var_dump(get_value(-2) / 2);
  var_dump(get_value(-4) / 2);

  // Divide by -2
  var_dump(get_value(16) / -2);
  var_dump(get_value(10) / -2);
  var_dump(get_value(8) / -2);
  var_dump(get_value(4) / -2);
  var_dump(get_value(2) / -2);
  var_dump(get_value(0) / -2);
  var_dump(get_value(-2) / -2);
  var_dump(get_value(-4) / -2);
}
