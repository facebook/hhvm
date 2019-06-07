<?hh

class C { public $a; }
<<__EntryPoint>> function main() {
$c = new C();
$c->a = function () use($c) {};
var_dump($c->a);

echo "===DONE===\n";
}
