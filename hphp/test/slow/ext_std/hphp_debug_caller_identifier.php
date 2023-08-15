<?hh

function foo() :mixed{
  return hphp_debug_caller_identifier();
}

function bar() :mixed{
  return foo();
}

class Baz {
  public static function smeth() :mixed{
    return foo();
  }

  public function imeth() :mixed{
    return foo();
  }
}
<<__EntryPoint>> function main(): void {
  $baz = new Baz();
  var_dump(bar());
  var_dump(Baz::smeth());
  var_dump($baz->imeth());
  $closure = () ==> foo();
  var_dump($closure());
}
