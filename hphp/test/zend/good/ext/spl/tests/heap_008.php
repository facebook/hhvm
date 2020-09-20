<?hh <<__EntryPoint>> function main(): void {
$h = new SplMaxHeap();

$h->insert(1);
$h->insert(5);
$h->insert(0);
$h->insert(4);

var_dump($h);
echo "===DONE===\n";
}
