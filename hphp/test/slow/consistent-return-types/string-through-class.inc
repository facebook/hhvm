<?hh

class C {}

class Base {
  function foo() : class<C> { return C::class; }
}

class Derived extends Base {
  function foo() : string {
    return __hhvm_intrinsics\launder_value("somestr");
  }
}

function mainImpl() : void {
  set_error_handler(
    (int $errno, string $errstr) ==> {
     if ($errno == E_WARNING) {
        var_dump("Warning: ".$errstr);
      } else if ($errno == E_NOTICE) {
        var_dump("Notice: ".$errstr);
      } else {
        throw new Exception($errstr);
      }
    }
  );

  try {
    __hhvm_intrinsics\debug_var_dump_lazy_class(new Derived()->foo());
  } catch (Exception $e) {
    var_dump("Error: ".$e->getMessage());
  }
}
