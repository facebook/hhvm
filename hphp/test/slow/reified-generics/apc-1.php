<?hh

class C<reify T> {
  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main() :mixed{
 $c = new C<int>();
 $c->f();

 apc_store('c', $c);
 $d = __hhvm_intrinsics\apc_fetch_no_check('c');

 $d->f();
}
