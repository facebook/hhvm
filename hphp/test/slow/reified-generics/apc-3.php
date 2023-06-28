<?hh

class C<reify T> {
  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

class D extends C<int> {}
<<__EntryPoint>> function main(): void {
$d = __hhvm_intrinsics\apc_fetch_no_check('d');
if ($d === false) {
  echo "Not in APC\n";
  $d = new D();
  $d->f();
  apc_store('d', $d);
} else {
  echo "In APC\n";
}
$d->f();
}
