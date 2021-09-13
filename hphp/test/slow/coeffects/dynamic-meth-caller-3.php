<?hh

class C {
  <<__DynamicallyCallable>>
  function foo()[defaults] { echo "in foo\n"; }
}

function pure_get()[] {
  return HH\dynamic_meth_caller('C', 'foo');
}

<<__EntryPoint>>
function main()[write_props] {
  $foo = pure_get();
  $foo(new C);
}
