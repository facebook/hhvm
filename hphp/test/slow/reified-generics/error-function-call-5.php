<?hh

class C {
  function f<reify Ta, reify Tb>() :mixed{}
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f<string>();
}
