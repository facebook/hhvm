<?hh

class C<reified T1> {
  function f<reified T2>(mixed $x) {
    var_dump($x is (T1, T2));
  }
}

$c = new C<reified string>();
$c->f<reified int>(tuple("hello", 1));
$c->f<reified int>(tuple(1, "hello"));
