<?hh

// Medium 2
function f() {
  echo "f\n";
  return varray[4, 5, 6];
}
<<__EntryPoint>>
function main_entry(): void {

  $array = varray[1, 2, 3];

  // Easy
  list($a, $b, $c) = $array;
  var_dump($a, $b, $c);

  // Medium
  $bucket = darray[];
  list($bucket[0], $bucket[1], $bucket[2]) = $array;
  var_dump($bucket);
  list($a, $b, $c) = f();
  var_dump($a, $b, $c);

  // Medium 3
  try {
    list($a, $b) = varray[];
    var_dump($a, $b);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  var_dump($a, $b);

  // Hard
  list($a, list(list($b), $c)) = varray[1, varray[varray[2], 3]];
  var_dump($a, $b, $c);

  // WTF
  $c = varray[1, 2, "derp"];
  list($a, $b, $c) = $c;
  var_dump($a, $b, $c);
}
