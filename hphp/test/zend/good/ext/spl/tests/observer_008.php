<?hh
class A extends SplObjectStorage { }
<<__EntryPoint>> function main(): void {
$o1 = new StdClass;
$o2 = new StdClass;
$o3 = new StdClass;

$a = new A;
$a->attach($o1);
$a->attach($o2);

$b = new SplObjectSTorage();
$b->attach($o2);
$b->attach($o3);

$a->addAll($b);

var_dump($a->count());

$a->detach($o3);
var_dump($a->count());

$a->removeAll($b);
var_dump($a->count());
echo "===DONE===\n";
}
