<?hh
function stats($f, $a) :mixed{
    $times = 90000;
    print "$f\n";
    ksort(inout $a);
    foreach($a as $k => $v)
        print "$k: $v: " . sprintf('%0.3f', $v / $times) . "\n";
}
<<__EntryPoint>> function main(): void {
$a = dict[];
$times = 90000;
for ($i = 0; $i < $times; $i++) {
    $p = range(1,4);
    shuffle(inout $p);
    $s = join('', $p);
    if (!($a[$s] ?? false)) $a[$s] = 0;
    $a[$s]++;
}

stats('shuffle', $a);
$a = dict[];
$times = 90000;
for ($i = 0; $i < $times; $i++) {
    $p = '1234';
    $s = str_shuffle($p);
    if (!($a[$s] ?? false)) $a[$s] = 0;
    $a[$s]++;
}

stats('str_shuffle', $a);
}
