<?hh <<__EntryPoint>> function main(): void {
$a = new SplMaxHeap;
$a->insert($a);
var_dump($a);
echo "===DONE===\n";
}
