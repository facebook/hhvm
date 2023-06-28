<?hh

function err_handler($errno, $errstr, $file, $line) :mixed{
  if ($errno === E_DEPRECATED) {
    echo 'E_DEPRECATED', ': ', $errstr, ' for ', $file, ':', $line, "\n";
    return true;
  } else if ($errno === E_USER_DEPRECATED) {
    echo 'E_USER_DEPRECATED', ': ', $errstr, ' for ', $file, ':', $line, "\n";
    return true;
  }
  throw new Exception($errstr);
}

trait Tr {
  <<__Deprecated('message')>>
  public function meth() :mixed{ echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public static function stMeth() :mixed{ echo __METHOD__, "\n"; }
}

class C {
  <<__Deprecated('message')>>
  public function meth() :mixed{ echo __METHOD__, "\n"; }
  <<__Deprecated('message')>>
  public static function stMeth() :mixed{ echo __METHOD__, "\n"; }
}

class C_T {
  use Tr;
}

<<__Deprecated('message')>>
function f() :mixed{ echo __METHOD__, "\n"; }
<<__Deprecated('message')>>
function f_gen() :AsyncGenerator<mixed,mixed,void>{ yield null; echo __METHOD__, "\n"; }
<<__Memoize, __Deprecated('message')>>
function fMem() :mixed{
  echo __METHOD__, "\n";
}

function basic() :mixed{
  echo '= ', __FUNCTION__, " =", "\n";
  f();
  C::stMeth();
  $inst = new C();
  $inst->meth();
  f_gen();
  echo "\n";
}

function memoized() :mixed{
  echo '= ', __FUNCTION__, " =", "\n";
  fMem();
  fMem();
  echo "\n";
}

function via_trait() :mixed{
  echo '= ', __FUNCTION__, " =", "\n";
  // f();
  C_T::stMeth();
  $inst = new C_T();
  $inst->meth();
  echo "\n";
}

function main() :mixed{
  basic();
  via_trait();
  memoized();
  echo 'Done', "\n";
}
<<__EntryPoint>>
function entrypoint_deprecated(): void {
  error_reporting(-1);
  set_error_handler(err_handler<>);

  main();
}
