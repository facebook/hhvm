<?hh

interface I { const x = ''; }
abstract class X implements I {
  abstract const int q;
  function foo() {
    return static::q;
  }
}
class Y extends X {
  const int q = 1;
  const x = 'foo';
}

<<__EntryPoint>>
function main_interface_override() {
var_dump(Y::foo());
}
