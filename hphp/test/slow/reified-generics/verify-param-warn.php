<?hh

class C {
}

function test<reify T>(<<__Soft>>int $foo) :mixed{
  return new T();
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(
    (int $errno,
    string $errstr,
    string $errfile,
    int $errline,
    darray $errcontext
    ) ==> {
      echo "ERROR: "; var_dump($errstr);
      return true;
    }
  );

  // Force prologue call.
  var_dump(test<C>(...__hhvm_intrinsics\launder_value(vec['notint'])));
}
