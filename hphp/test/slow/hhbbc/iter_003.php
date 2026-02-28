<?hh

class A { function heh() :mixed{ echo "heh\n"; } }
function foo() :mixed{
  $x = dict['foo' => new A, 'bar' => new A];
  foreach ($x as $k => $v) {
    var_dump($k);
    var_dump($v);
  }
}

<<__EntryPoint>>
function main_iter_003() :mixed{
foo();
}
