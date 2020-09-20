<?hh

class Foo {}

<<__EntryPoint>>
function main(): void {
  $f = new Foo();
  $wr = new WeakRef(__hhvm_intrinsics\launder_value($f));
  $f = null;
  __hhvm_intrinsics\launder_value(heapgraph_create());
  __hhvm_intrinsics\launder_value($wr->valid());
  echo "DONE";
}
