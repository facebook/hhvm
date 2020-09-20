<?hh
function fact($x) {
  if($x <= 1) {
    return 1;
  }

  return gmp_mul($x, fact($x-1));
}


<<__EntryPoint>>
function main_002() {
print gmp_strval(fact(218)) . "\n";
}
