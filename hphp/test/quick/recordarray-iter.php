<?hh

class Bar {
  public int $f = 42;
}

record Foo {
  int x;
  Bar z;
}

<<__EntryPoint>>
function main() {
  $o = new Bar;
  $a = Foo@['x' => 10, 'z' => $o];
  $a['y'] = 'abc';
  foreach ($a as $k => $v) {
    var_dump($k);
    var_dump($v);
  }
}
