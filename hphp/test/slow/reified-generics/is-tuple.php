<?hh

class C<reify T1> {
  function f<reify T2>(mixed $x) :mixed{
    var_dump($x is (T1, T2));
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<string>();
$c->f<int>(tuple("hello", 1));
$c->f<int>(tuple(1, "hello"));
}
