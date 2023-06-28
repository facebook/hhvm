<?hh

function foo<reify T>() :mixed{
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

<<__EntryPoint>>
function main() :mixed{
  foo<int>();

  $foo = foo<int>;
  apc_store('foo', $foo);

  $retrieve = __hhvm_intrinsics\apc_fetch_no_check('foo');
  $retrieve();
}
