<?hh

// Prevent constant folding
function get_value(int $val) :int{
  return HH\Lib\PseudoRandom\int($val, $val);
}

<<__EntryPoint>>
function main(): void {
  var_dump(get_value(-1) / -1);
  var_dump(get_value(1) / 1);
  var_dump(get_value(2) / 2);

  var_dump(get_value(-1) / -1.0);
  var_dump(get_value(1) / 1.0);
}
