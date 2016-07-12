<?hh

function foo() {
  return hphp_debug_caller_info();
}

function bar() {
  return foo();
}

class Baz {
  public static function smeth() {
    return foo();
  }

  public function imeth() {
    return foo();
  }
}

$baz = new Baz();
var_dump(bar());
var_dump(Baz::smeth());
var_dump(Baz::imeth());
var_dump($baz->smeth());
var_dump($baz->imeth());
