<?hh

class C {}

class Base {
  function foo() : classname<C> { return nameof C; }
}

class Derived1 extends Base {
  function foo() : class<C> {
    return __hhvm_intrinsics\launder_value(
      HH\classname_to_class(C::class)
    );
  }
}

class Derived2 extends Base {
  function foo() : class<C> {
    return __hhvm_intrinsics\launder_value(C::class);
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
    __hhvm_intrinsics\debug_var_dump_lazy_class(new Derived1()->foo());
  } catch (Exception $e) {
    var_dump("Error: ".$e->getMessage());
  }
  try {
    __hhvm_intrinsics\debug_var_dump_lazy_class(new Derived2()->foo());
  } catch (Exception $e) {
    var_dump("Error: ".$e->getMessage());
  }
}

