<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\getType<T>());
  }
}

class D extends C<int> {}
<<__EntryPoint>> function main(): void {
$d = apc_fetch('d');
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
