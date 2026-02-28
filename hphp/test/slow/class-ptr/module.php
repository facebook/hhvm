<?hh

<<__EntryPoint>>
function main(): void {
  include 'module.inc';

  // lazy class CInternal::class has been implicitly coerced to string
  try {
    $c = HH\classname_to_class(CPublic::in_str_implicit());
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }

  // explicit string of the CInternal class name
  try {
    $c = HH\classname_to_class(CPublic::in_str());
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }

  // lazy class admitted
  $c = HH\classname_to_class(CPublic::in_lazy());
  __hhvm_intrinsics\debug_var_dump_lazy_class($c);

  // safe pointer
  $ptr = CPublic::in_ptr();
  __hhvm_intrinsics\debug_var_dump_lazy_class($ptr);
  $d = HH\classname_to_class($ptr);
  __hhvm_intrinsics\debug_var_dump_lazy_class($d);
}
