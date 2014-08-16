<?hh

// TODO: Make varray support sorts that renumber the keys 0 thru n-1
// (i.e. sort(), usort(), etc) while still warning for associative-style
// sorts (i.e. asort(), ksort(), uasort(), etc).

function main() {
  $a = varray();
  $a[0] = 1;
  $a[1] = 0;
  sort($a);
  var_dump($a);
}

main();
