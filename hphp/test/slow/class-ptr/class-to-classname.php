<?hh

class C {}

<<__EntryPoint>>
function main(): void {
  $w = HH\class_to_classname("C");
  __hhvm_intrinsics\debug_var_dump_lazy_class($w);
  $x = HH\class_to_classname(trim("C "));
  __hhvm_intrinsics\debug_var_dump_lazy_class($x);

  $y = C::class;
  $y = HH\class_to_classname($y);
  __hhvm_intrinsics\debug_var_dump_lazy_class($y);

  $z = __hhvm_intrinsics\create_class_pointer(C::class);
  $z = HH\class_to_classname($z);
  __hhvm_intrinsics\debug_var_dump_lazy_class($z);

  HH\class_to_classname(3);
}
