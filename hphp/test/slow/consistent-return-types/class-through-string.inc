<?hh

class C {}

class BazObj { function __toString() : string { return "baz"; } }

class Base {
  function foo() : string { return "foo"; }
  function bar() : Stringish { return "bar"; }
  function baz() : StringishObject { return new BazObj(); }
}

class Derived1 extends Base {
  function foo() : class<C> {
    return __hhvm_intrinsics\launder_value(C::class);
  }

  function bar() : class<C> {
    return __hhvm_intrinsics\launder_value(C::class);
  }

  function baz() : class<C> {
    return __hhvm_intrinsics\launder_value(C::class);
  }
}

class Derived2 extends Base {
  function foo() : class<C> {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
  }

  function bar() : class<C> {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
  }

  function baz() : class<C> {
    return __hhvm_intrinsics\launder_value(HH\classname_to_class(C::class));
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
    var_dump((new Derived1())->foo());
    var_dump((new Derived1())->bar());
    var_dump((new Derived1())->baz());
  } catch (Exception $e) {
    var_dump("Error: ".$e->getMessage());
  }
  try {
    var_dump((new Derived2())->foo());
    var_dump((new Derived2())->bar());
    var_dump((new Derived2())->baz());
  } catch (Exception $e) {
    var_dump("Error: ".$e->getMessage());
  }
}
