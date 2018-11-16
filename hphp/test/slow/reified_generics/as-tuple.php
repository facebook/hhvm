<?hh

class C<reify T1> {
  function f<reify T2>(mixed $x) {
    try {
      $x as (T1, T2);
      var_dump("yep");
    } catch (Exception $_) {
      var_dump("nope");
    }
  }
}

$c = new C<reify string>();
$c->f<reify int>(tuple("hello", 1));
$c->f<reify int>(tuple(1, "hello"));
