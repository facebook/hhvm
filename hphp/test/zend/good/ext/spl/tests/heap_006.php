<?hh <<__EntryPoint>> function main(): void {
$input = range(1,100);
shuffle(inout $input);

$h = new SplMaxHeap();

foreach($input as $i) {
    $h->insert($i);
}

foreach ($h as $k => $o) {
    echo "$k => $o\n";
}
echo "===DONE===\n";
}
