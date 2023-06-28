<?hh

class Foo<T1 as string, T2 as int> {
  function foo1<T1>(T1 $x, T2 $y) :mixed{
    var_dump($x);
    var_dump($y);
  }
  function foo2<T2, T1>(T1 $x, T2 $y) :mixed{
    var_dump($x);
    var_dump($y);
  }
  function foo3<T2, T1 as int>(T1 $x, T2 $y) :mixed{
    var_dump($x);
    var_dump($y);
  }
}

<<__EntryPoint>>
function main() :mixed{
  $o = new Foo;
  $o->foo1(1, 1); // no error, T1 shadowed
  $o->foo1(1, 'a'); // T2 warn, not shadowed
  $o->foo2(1, 1); // no error, both shadowed
  $o->foo3(1, 1); // no error, both shadowed
  $o->foo3('a', 1); // warn on local T1, T2 shadowed
}
