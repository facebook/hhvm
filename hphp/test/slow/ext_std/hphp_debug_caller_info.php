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
<<__EntryPoint>> function main(): void {
$baz = new Baz();
var_dump(bar());
var_dump(Baz::smeth());
var_dump($baz->imeth());
}
