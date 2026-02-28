<?hh

class C<reify T1> {
  function f<reify T2>(mixed $x) :mixed{
    try {
      $x as (T1, T2);
      var_dump("yep");
    } catch (Exception $_) {
      var_dump("nope");
    }
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<string>();
$c->f<int>(tuple("hello", 1));
$c->f<int>(tuple(1, "hello"));
}
