<?hh

class C {}

<<__EntryPoint>>
function main(): void {
  echo "Lazy class: ";
  __hhvm_intrinsics\debug_var_dump_lazy_class(C::class);
  echo "Class: ";
  __hhvm_intrinsics\debug_var_dump_lazy_class(
    __hhvm_intrinsics\create_class_pointer(C::class)
  );
  echo "String: ";
  __hhvm_intrinsics\debug_var_dump_lazy_class("C");
}
