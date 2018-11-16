<?hh

class C<reify T1> {
  function f<reify T2>(mixed $x) {
    var_dump($x is (T1, T2));
  }
}

$c = new C<reify string>();
$c->f<reify int>(tuple("hello", 1));
$c->f<reify int>(tuple(1, "hello"));
