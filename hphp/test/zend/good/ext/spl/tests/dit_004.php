<?hh <<__EntryPoint>> function main(): void {
$a = new DirectoryIterator(__DIR__);
$b = clone $a;
var_dump($b->__toString() == $a->__toString());
var_dump($a->key(), $b->key());
$a->next();
$a->next();
$a->next();
$c = clone $a;
var_dump($c->__toString() == $a->__toString());
var_dump($a->key(), $c->key());
echo "===DONE===\n";
}
