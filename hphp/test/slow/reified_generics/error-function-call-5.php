<?hh

class C {
  function f<reify Ta, reify Tb>() {}
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f<string>();
}
