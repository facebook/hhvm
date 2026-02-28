<?hh <<__EntryPoint>> function main(): void {
$i = 0;
$a = vec[];
$b = vec[];
while ($i++ < 1000) {
    $a[] = $i;
    $b[] = (string)$i;
}
$s1 = array_sum($a);
$s2 = array_sum($b);
var_dump($s1, $s2);

$j = 0;
$c = vec[];
$d = vec[];
while ($j++ < 100000) {
    $c[] = $j;
    $d[] = (string) $j;
}
$s3 = array_sum($c);
$s4 = array_sum($d);
var_dump($s3, $s4);
}
