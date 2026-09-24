<?hh

type ScalarAlias = int;

function native_dynamic_name(string $name): string {
  return __hhvm_intrinsics\launder_value($name)."";
}

function native_attempt(
  string $label,
  (function(): void) $operation,
): void {
  echo $label."\n";
  try {
    $operation();
    echo "ok\n";
  } catch (Throwable $exception) {
    echo "error: ".$exception->getMessage()."\n";
  }
}

<<__EntryPoint>>
function native_apis_main(): void {
  require_once __DIR__."/notice/Targets.inc.php";
  require_once __DIR__."/enforced/Targets.inc.php";
  require_once __DIR__."/loose/Target.inc.php";
  require_once __DIR__."/families/implicit/member/Target.inc.php";

  set_error_handler((int $_errno, string $message) ==> {
    echo "notice: ".$message."\n";
    return true;
  });

  native_attempt("dynamic:notice", () ==> {
    HH\dynamic_class_meth(
      native_dynamic_name(nameof StrictNotice\Target),
      "method",
    );
  });
  native_attempt("dynamic:constant-string", () ==> {
    $class = "StrictNotice\\Target";
    HH\dynamic_class_meth($class, "method");
  });
  native_attempt("dynamic:enforced", () ==> {
    HH\dynamic_class_meth(
      native_dynamic_name(nameof StrictEnforced\Target),
      "method",
    );
  });
  native_attempt("dynamic:allowed", () ==> {
    HH\dynamic_class_meth(
      native_dynamic_name(nameof StrictEnforced\Allowed),
      "method",
    );
  });
  native_attempt("dynamic:soft", () ==> {
    HH\dynamic_class_meth(
      native_dynamic_name(nameof StrictEnforced\Soft),
      "method",
    );
  });
  native_attempt("dynamic:implicit", () ==> {
    HH\dynamic_class_meth(
      native_dynamic_name(nameof ImplicitMember\Target),
      "method",
    );
  });
  native_attempt("dynamic:loose", () ==> {
    HH\dynamic_class_meth(
      native_dynamic_name(nameof Loose\Target),
      "method",
    );
  });
  native_attempt("dynamic:lazy-class", () ==> {
    HH\dynamic_class_meth(StrictNotice\Target::class, "method");
  });
  native_attempt("dynamic:class", () ==> {
    $class = HH\get_class_from_object(new StrictNotice\Target());
    HH\dynamic_class_meth($class, "method");
  });
  native_attempt("dynamic:unknown", () ==> {
    HH\dynamic_class_meth(native_dynamic_name("MissingClass"), "method");
  });

  native_attempt("type-structure:notice", () ==> {
    HH\type_structure(native_dynamic_name(nameof StrictNotice\Target), "T");
  });
  native_attempt("type-structure:constant-string", () ==> {
    HH\type_structure("StrictNotice\\Target", "T");
  });
  native_attempt("type-structure-no-throw:notice", () ==> {
    HH\type_structure_no_throw(
      native_dynamic_name(nameof StrictNotice\Target),
      "T",
    );
  });
  native_attempt("type-structure-classname:notice", () ==> {
    HH\type_structure_classname(
      native_dynamic_name(nameof StrictNotice\Target),
      "CLASS_TYPE",
    );
  });
  native_attempt("type-structure-class:notice", () ==> {
    HH\type_structure_class(
      native_dynamic_name(nameof StrictNotice\Target),
      "CLASS_TYPE",
    );
  });

  native_attempt("type-structure:enforced", () ==> {
    HH\type_structure(native_dynamic_name(nameof StrictEnforced\Target), "T");
  });
  native_attempt("type-structure-no-throw:enforced", () ==> {
    HH\type_structure_no_throw(
      native_dynamic_name(nameof StrictEnforced\Target),
      "T",
    );
  });
  native_attempt("type-structure-classname:enforced", () ==> {
    HH\type_structure_classname(
      native_dynamic_name(nameof StrictEnforced\Target),
      "CLASS_TYPE",
    );
  });
  native_attempt("type-structure-class:enforced", () ==> {
    HH\type_structure_class(
      native_dynamic_name(nameof StrictEnforced\Target),
      "CLASS_TYPE",
    );
  });
  native_attempt("type-structure:soft", () ==> {
    HH\type_structure(native_dynamic_name(nameof StrictEnforced\Soft), "T");
  });
  native_attempt("type-structure:allowed", () ==> {
    HH\type_structure(native_dynamic_name(nameof StrictEnforced\Allowed), "T");
  });

  native_attempt("type-structure:lazy-class", () ==> {
    HH\type_structure(StrictNotice\Target::class, "T");
  });
  native_attempt("type-structure:class", () ==> {
    $class = HH\get_class_from_object(new StrictNotice\Target());
    HH\type_structure($class, "T");
  });
  native_attempt("type-structure:object", () ==> {
    HH\type_structure(new StrictNotice\Target(), "T");
  });
  native_attempt("type-structure:alias", () ==> {
    HH\type_structure(native_dynamic_name("ScalarAlias"));
  });
}
