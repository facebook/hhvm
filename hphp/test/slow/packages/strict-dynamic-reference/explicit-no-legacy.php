<?hh

<<__EntryPoint>>
function main(): void {
  require_once __DIR__."/notice/Targets.inc.php";

  set_error_handler((int $_errno, string $message) ==> {
    echo "notice: ".$message."\n";
    return true;
  });

  $class = __hhvm_intrinsics\launder_value(nameof StrictNotice\Target)."";
  HH\classname_to_class($class);
  echo "ok\n";
}
