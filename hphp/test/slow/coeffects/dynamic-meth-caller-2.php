<?hh

class C {
  <<__DynamicallyCallable>>
  function foo()[write_props] { echo "in foo\n"; }
}

function pure_get()[] {
  return HH\dynamic_meth_caller('C', 'foo');
}

<<__EntryPoint>>
function main() {
  $foo = pure_get();
  $foo(new C);
}
