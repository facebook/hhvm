<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main() {
 $c = new C<int>();
 $c->f();

 apc_store('c', $c);
 $d = __hhvm_intrinsics\apc_fetch_no_check('c');

 $d->f();
}
