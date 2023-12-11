<?hh
<<__EntryPoint>>
function entrypoint_simple(): void {


  error_reporting(0);

  list($a, $b) = vec[1, 2];
  var_dump($a);
  var_dump($b);

  $array = vec[];
  list($array[], $array[], $array[]) = vec[1, 2, 3];
  var_dump($array);

  $a = vec[1, 2];
  list($a, $b) = $a;
  var_dump($a);
  var_dump($b);

  $e = vec[0,0];
  $f = 0;
  $g1 = vec[10,11];
  $g2 = vec[20,21];
  $g3 = vec[30,31];
  $g = vec[$g1,$g2,$g3];
  list($e[$f++],$e[$f++]) = $g[$f];
  var_dump($e);
  $h = vec[1, 2, 3];
  $i = 0;
  $j = dict[];
  $j[$i++] = $h[$i++];
  var_dump($j);
}
