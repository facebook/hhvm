<?hh

class A<reify Ta, reify Tb> {}

class C<reify Ta> {
  function f<reify Tb>(A<Ta, Tb> $_) :mixed{}
}
<<__EntryPoint>> function main(): void {
$c = new C<int>();
$c->f<string>(new A<int, string>());
$c->f<int>(new A<int, string>());
}
