<?hh

class A {
  function c() {
    return tuple(1,2);
  }
}

<<__EntryPoint>>
function main_tuple() {
var_dump((new A)->c());
}
