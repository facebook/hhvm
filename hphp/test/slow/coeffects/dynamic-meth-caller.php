<?hh

class A {
  const ctx C = [write_props];
  <<__DynamicallyCallable>>
  function f()[write_props] :mixed{ echo "in A::f\n"; }
}

function static_fn()[write_props] :mixed{
  $f1 = meth_caller(A::class, 'f');
  $f1(new A);

  $f2 = HH\dynamic_meth_caller('A', 'f');
  $f2(new A);
}

function poly_fn(mixed $x)[$x::C] :mixed{
  $f1 = meth_caller(A::class, 'f');
  $f1(new A);

  $f2 = HH\dynamic_meth_caller('A', 'f');
  $f2(new A);
}


<<__EntryPoint>>
function main() :mixed{
  static_fn();
  poly_fn(new A);
}
