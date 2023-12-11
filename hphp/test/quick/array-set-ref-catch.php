<?hh

// generate a string from bits in i where 0=>a, 1=>A. This causes
// hash collisions if the string hash is case-insensitive.
function make_key($i) :mixed{
  for ($s = ""; $i != 0; $i >>= 1) $s .= ($i & 1) ? "A" : "a";
  return $s;
}

function test(inout $a, $b) :mixed{
  for ($i = 0; $i < 40000; $i++) {
    $a[make_key($i)] = $b;
  }
  var_dump($a);
}
<<__EntryPoint>> function main(): void {
$y = dict[];
test(inout $y, 5);
}
