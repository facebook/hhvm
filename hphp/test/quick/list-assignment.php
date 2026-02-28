<?hh

// Medium 2
function f() :mixed{
  echo "f\n";
  return vec[4, 5, 6];
}
<<__EntryPoint>>
function main_entry(): void {

  $array = vec[1, 2, 3];

  // Easy
  list($a, $b, $c) = $array;
  var_dump($a, $b, $c);

  // Medium
  $bucket = dict[];
  list($bucket[0], $bucket[1], $bucket[2]) = $array;
  var_dump($bucket);
  list($a, $b, $c) = f();
  var_dump($a, $b, $c);

  // Medium 3
  try {
    list($a, $b) = vec[];
    var_dump($a, $b);
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
  var_dump($a, $b);

  // Hard
  list($a, list(list($b), $c)) = vec[1, vec[vec[2], 3]];
  var_dump($a, $b, $c);

  // WTF
  $c = vec[1, 2, "derp"];
  list($a, $b, $c) = $c;
  var_dump($a, $b, $c);
}
