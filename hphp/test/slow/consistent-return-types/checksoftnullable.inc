<?hh

class Base {
  function foo() : <<__Soft>> ?int {
    return 1;
  }

  function bar() : int {
    return 1;
  }
}

class Derived extends Base {
  function foo() : int {
    return 'str';
  }

  function bar() : <<__Soft>> ?int {
    return 'str';
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
    var_dump(new Derived()->foo());
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }

  try {
    var_dump(new Derived()->bar());
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
