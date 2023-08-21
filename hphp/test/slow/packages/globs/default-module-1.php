<?hh

class C {
  function m(): void {}
}
function f(): void {}

<<__EntryPoint>>
function main() : void {
  var_dump((new ReflectionClass('C'))->getModule());
  var_dump((new ReflectionFunction('f'))->getModule());
  var_dump((new ReflectionMethod('C', 'm'))->getModule());
}
