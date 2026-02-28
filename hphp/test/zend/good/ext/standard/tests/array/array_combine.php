<?hh
<<__EntryPoint>> function main(): void {
$array1 = vec['green', 'red', 'yellow'];
$array2 = vec['1', '2', '3'];
$array3 = vec[0, 1, 2];
$array4 = vec[TRUE, FALSE, NULL];
$a = array_combine($array1, $array1);
$b = array_combine($array1, $array2);
$c = array_combine($array1, $array3);
$d = array_combine($array1, $array4);
$e = array_combine($array2, $array1);
$f = array_combine($array2, $array2);
$g = array_combine($array2, $array3);
$h = array_combine($array2, $array4);
$i = array_combine($array3, $array1);
$j = array_combine($array3, $array2);
$k = array_combine($array3, $array3);
$l = array_combine($array3, $array4);
$m = array_combine($array4, $array1);
$n = array_combine($array4, $array2);
$o = array_combine($array4, $array3);
$p = array_combine($array4, $array4);
print_r($a);
print_r($b);
print_r($c);
print_r($d);
print_r($e);
print_r($f);
print_r($g);
print_r($h);
print_r($i);
print_r($j);
print_r($k);
print_r($l);
print_r($m);
print_r($n);
print_r($o);
print_r($p);
}
