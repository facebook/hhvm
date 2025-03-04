<?hh

<<__EntryPoint>>
function main(): mixed {
  var_dump(gmp_pow(gmp_intval(10), 1 << 33));
}
