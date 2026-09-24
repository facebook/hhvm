<?hh

<<__EntryPoint>>
function native_force_main(): void {
  require_once __DIR__."/notice/Targets.inc.php";
  set_error_handler((int $_errno, string $message) ==> {
    echo "notice: ".$message."\n";
    return true;
  });

  $class = __hhvm_intrinsics\launder_value(nameof StrictNotice\Target)."";
  HH\dynamic_class_meth_force($class, "undynamic");
  echo "ok\n";
}
