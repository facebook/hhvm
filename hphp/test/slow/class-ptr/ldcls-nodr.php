<?hh

class A {}

function f(string $s): void {
  $p = HH\classname_to_class($s);
  __hhvm_intrinsics\debug_var_dump_lazy_class($p);
}

<<__EntryPoint>>
function main(): void {
  f("A");
}
