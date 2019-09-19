<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main() {
 $c = __hhvm_intrinsics\apc_fetch_no_check('c');
 if ($c === false) {
   echo "Not in APC\n";
   $c = new C<int>();
   $c->f();
   apc_store('c', $c);
 } else {
   echo "In APC\n";
 }
 $c->f();
}
