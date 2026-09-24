<?hh

function dynamic_name(string $name): string {
  return __hhvm_intrinsics\launder_value($name)."";
}

function attempt(string $label, (function(): void) $operation): void {
  echo $label."\n";
  try {
    $operation();
    echo "ok\n";
  } catch (Throwable $exception) {
    echo "error: ".$exception->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  require_once __DIR__."/strict/Target.inc.php";

  set_error_handler((int $_errno, string $message) ==> {
    echo "notice: ".$message."\n";
    return true;
  });

  attempt("ordinary", () ==> {
    HH\classname_to_class(dynamic_name(nameof StrictBackdoor\Target))
      |> $$::run();
  });
  attempt("strict-isolation-backdoor", () ==> {
    HH\classname_to_class_strict_isolation_backdoor(
      dynamic_name(nameof StrictBackdoor\Target),
    ) |> $$::run();
  });
  attempt("strict-isolation-backdoor-missing", () ==> {
    HH\classname_to_class_strict_isolation_backdoor(
      dynamic_name(nameof StrictBackdoor\Missing),
    ) |> $$::run();
  });
  attempt("strict-isolation-backdoor-soft", () ==> {
    HH\classname_to_class_strict_isolation_backdoor(
      dynamic_name(nameof StrictBackdoor\Soft),
    ) |> $$::run();
  });
}
