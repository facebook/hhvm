<?hh

class Dtor {
}

class Foo { public $bug; };

function main() {
  $x = new Foo;
  $x->bug = new Dtor;
  $x->bug += 12;
  var_dump($x);
}
<<__EntryPoint>> function main_entry() {
main();
echo "done\n";
}
