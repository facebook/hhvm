<?hh

class Foo {
  public function __get($x) {
    var_dump("getter: " . $x);
    if ($x == 'foo') return 42;
    $this->foo++;
    $this->asd = new stdclass;
  }
}

function main() {
  $x = new Foo;
  $x->asd++;
  var_dump($x);
}
<<__EntryPoint>> function main_entry() {
main();
echo "Done\n";
}
