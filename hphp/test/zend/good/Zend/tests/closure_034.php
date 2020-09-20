<?hh

class C { public $a; }
<<__EntryPoint>> function main(): void {
$c = new C();
$c->a = function () use($c) {};
var_dump($c->a);

echo "===DONE===\n";
}
