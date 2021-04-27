<?hh

class A {
  function f()[rx] {
  }
}

<<__EntryPoint>>
function main()[write_props] {
  $f = meth_caller('A', 'f');
  $f(new A);
}
