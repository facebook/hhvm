<?hh


error_reporting(0);

list($a, $b) = varray[1, 2];
var_dump($a);
var_dump($b);

$array = varray[];
list($array[], $array[], $array[]) = varray[1, 2, 3];
var_dump($array);

$a = varray[1, 2];
list($a, $b) = $a;
var_dump($a);
var_dump($b);

$e = varray[0,0];
$f = 0;
$g1 = varray[10,11];
$g2 = varray[20,21];
$g3 = varray[30,31];
$g = varray[$g1,$g2,$g3];
list($e[$f++],$e[$f++]) = $g[$f];
var_dump($e);
$h = varray[1, 2, 3];
$i = 0;
$j = varray[];
$j[$i++] = $h[$i++];
var_dump($j);
