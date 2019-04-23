<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\getType<T>());
  }
}

<<__EntryPoint>>
function main() {
 $c = apc_fetch('c');
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
