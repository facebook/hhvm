<?hh

class C<reified T1> {
  function f<reified T2>(mixed $x) {
    try {
      $x as (T1, T2);
      var_dump("yep");
    } catch (Exception $_) {
      var_dump("nope");
    }
  }
}

$c = new C<reified string>();
$c->f<reified int>(tuple("hello", 1));
$c->f<reified int>(tuple(1, "hello"));
