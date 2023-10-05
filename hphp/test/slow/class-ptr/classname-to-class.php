<?hh

function v(mixed $m): void {
  __hhvm_intrinsics\debug_var_dump_lazy_class($m);
}
<<__DynamicallyReferenced>> class C {}
class D {}

<<__EntryPoint>>
function main(): void {
  $w = HH\classname_to_class("C");
  __hhvm_intrinsics\debug_var_dump_lazy_class($w);
  $x = HH\classname_to_class(trim("D "));
  __hhvm_intrinsics\debug_var_dump_lazy_class($x);

  $y = C::class;
  __hhvm_intrinsics\debug_var_dump_lazy_class($y);
  $y = HH\classname_to_class($y);
  __hhvm_intrinsics\debug_var_dump_lazy_class($y);

  $z = __hhvm_intrinsics\create_class_pointer(D::class);
  $zz = HH\classname_to_class($z);
  invariant($z === $zz, "Should be same");
}
