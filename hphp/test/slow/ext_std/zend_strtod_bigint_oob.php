<?hh
// Regression: a ~295KB significant-digit string drives the dtoa Bigint size
// class past the old 16-entry Balloc/Bfree freelist. Parsing must not read or
// write out of bounds (CVE-2009-0689 class; T277968123). Under ASan this faults
// without the freelist removal and is clean with it.
<<__EntryPoint>>
function main(): void {
  $s = "1.".str_repeat("1", 294912);
  var_dump(is_numeric($s));
  $f = (float)$s;
  var_dump($f > 1.1 && $f < 1.2);
}
