<?hh

class Foo {
  public function __get($x) {
    var_dump("getter: " . $x);
    if ($x == 'foo') return 42;
    $this->foo++;
    $this->asd++;
  }
}

<<__EntryPoint>> function main(): void {
  $x = new Foo;
  $x->asd++;
  var_dump($x);
}
