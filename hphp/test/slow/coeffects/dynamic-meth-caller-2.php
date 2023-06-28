<?hh

class C {
  <<__DynamicallyCallable>>
  function foo()[write_props] :mixed{ echo "in foo\n"; }
}

function pure_get()[] :mixed{
  return HH\dynamic_meth_caller('C', 'foo');
}

<<__EntryPoint>>
function main() :mixed{
  $foo = pure_get();
  $foo(new C);
}
