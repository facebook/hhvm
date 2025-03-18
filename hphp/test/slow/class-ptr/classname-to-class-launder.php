<?hh

function v(mixed $m): void {
  __hhvm_intrinsics\debug_var_dump_lazy_class($m);
}
<<__DynamicallyReferenced>> class C {}
class D {}
<<__DynamicallyReferenced(1)>> class E {}
<<__DynamicallyReferenced(0)>> class F {}

<<__EntryPoint>>
function main(): void {
  $w = HH\classname_to_class(
    __hhvm_intrinsics\launder_value("C")
  );
  __hhvm_intrinsics\debug_var_dump_lazy_class($w);
  $x = HH\classname_to_class(
    __hhvm_intrinsics\launder_value(trim("D "))
  );
  __hhvm_intrinsics\debug_var_dump_lazy_class($x);

  $y = C::class;
  __hhvm_intrinsics\debug_var_dump_lazy_class($y);
  $y = HH\classname_to_class(
    __hhvm_intrinsics\launder_value($y)
  );
  __hhvm_intrinsics\debug_var_dump_lazy_class($y);

  $z = HH\classname_to_class(D::class);
  $zz = HH\classname_to_class(
    __hhvm_intrinsics\launder_value($z)
  );
  invariant($z === $zz, "Should be same");

  $_ = HH\classname_to_class(
    __hhvm_intrinsics\launder_value(nameof E)
  );
  $_ = HH\classname_to_class(
    __hhvm_intrinsics\launder_value(nameof F)
  );
}
