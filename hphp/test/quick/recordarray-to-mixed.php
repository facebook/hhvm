<?hh

class Bar {
  public int $f = 42;
}

record FooBar {
  Bar z;
}

<<__EntryPoint>>
function main() {
  $o1 = new Bar;
  $o2 = new Bar;
  $o2->f = 10;
  $r1 = FooBar@['z' => $o1];
  $key = 'abc'.count(varray[1,2]);
  $r1[$key] = 42;
  $r2 = $r1;
  $r1[1] = $o2;
  $r1['z'] = $o2;
  var_dump($r1);
  var_dump($r2['z']);
}
