<?php
function stats($f, $a) {
    $times = 90000;
    print "$f\n";
    ksort($a);
    foreach($a as $k => $v)
        print "$k: $v: " . sprintf('%0.3f', $v / $times) . "\n";
}
$a = array();
$times = 90000;
for ($i = 0; $i < $times; $i++) {
    $p = range(1,4);
    shuffle($p);
    $s = join('', $p);
    if (empty($a[$s])) $a[$s] = 0; 
    $a[$s]++;
}

stats('shuffle', $a);
$a = array();
$times = 90000;
for ($i = 0; $i < $times; $i++) {
    $p = '1234';
    $s = str_shuffle($p);
    if (empty($a[$s])) $a[$s] = 0;
    $a[$s]++;
}

stats('str_shuffle', $a);
?>