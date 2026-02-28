<?hh

// Prevent constant folding
function get_value(int $val) :int{
  return HH\Lib\PseudoRandom\int($val, $val);
}

<<__EntryPoint>>
function main(): void {
  var_dump(get_value(-1) / 1);
  var_dump(get_value(0) / 1);
  var_dump(get_value(1) / 1);
  var_dump(get_value(2) / 1);

  var_dump(get_value(-1) / -1);
  var_dump(get_value(0) / -1);
  var_dump(get_value(1) / -1);
  var_dump(get_value(2) / -1);

  var_dump(get_value(-1) / 1.0);
  var_dump(0.0 / get_value(1));
  var_dump(get_value(1) / 1.0);
}
