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
  require_once __DIR__."/notice/Targets.inc.php";
  require_once __DIR__."/enforced/Targets.inc.php";
  require_once __DIR__."/loose/Target.inc.php";
  require_once __DIR__."/families/implicit/member/Target.inc.php";

  set_error_handler((int $_errno, string $message) ==> {
    echo "notice: ".$message."\n";
    return true;
  });

  attempt("notice:new", () ==> {
    $class = dynamic_name(nameof StrictNotice\Target);
    new $class();
  });
  attempt("notice:constant", () ==> {
    $class = dynamic_name(nameof StrictNotice\Target);
    $_ = $class::VALUE;
  });
  attempt("notice:property", () ==> {
    $class = dynamic_name(nameof StrictNotice\Target);
    $_ = $class::$value;
  });
  attempt("notice:method", () ==> {
    $class = dynamic_name(nameof StrictNotice\Target);
    $class::method();
  });
  attempt("notice:soft-method", () ==> {
    $class = dynamic_name(nameof StrictNotice\Soft);
    $class::method();
  });
  attempt("notice:allowed-method", () ==> {
    $class = dynamic_name(nameof StrictNotice\Allowed);
    $class::method();
  });
  attempt("notice:classname", () ==> {
    HH\classname_to_class(dynamic_name(nameof StrictNotice\Target));
  });
  attempt("notice:allowed-classname", () ==> {
    HH\classname_to_class(dynamic_name(nameof StrictNotice\Allowed));
  });
  attempt("notice:soft-classname", () ==> {
    HH\classname_to_class(dynamic_name(nameof StrictNotice\Soft));
  });

  attempt("enforced:new", () ==> {
    $class = dynamic_name(nameof StrictEnforced\Target);
    new $class();
  });
  attempt("enforced:constant", () ==> {
    $class = dynamic_name(nameof StrictEnforced\Target);
    $_ = $class::VALUE;
  });
  attempt("enforced:property", () ==> {
    $class = dynamic_name(nameof StrictEnforced\Target);
    $_ = $class::$value;
  });
  attempt("enforced:method", () ==> {
    $class = dynamic_name(nameof StrictEnforced\Target);
    $class::method();
  });
  attempt("enforced:classname", () ==> {
    HH\classname_to_class(dynamic_name(nameof StrictEnforced\Target));
  });

  attempt("enforced:allowed", () ==> {
    $class = dynamic_name(nameof StrictEnforced\Allowed);
    $class::method();
  });
  attempt("enforced:allowed-classname", () ==> {
    HH\classname_to_class(dynamic_name(nameof StrictEnforced\Allowed));
  });
  attempt("enforced:same-package-classname", () ==> {
    StrictEnforced\same_package_classname_to_class();
  });
  attempt("enforced:soft-method", () ==> {
    $class = dynamic_name(nameof StrictEnforced\Soft);
    $class::method();
  });
  attempt("enforced:soft-classname", () ==> {
    HH\classname_to_class(dynamic_name(nameof StrictEnforced\Soft));
  });

  attempt("implicit:method", () ==> {
    $class = dynamic_name(nameof ImplicitMember\Target);
    $class::method();
  });
  attempt("loose:method", () ==> {
    $class = dynamic_name(nameof Loose\Target);
    $class::method();
  });
}
