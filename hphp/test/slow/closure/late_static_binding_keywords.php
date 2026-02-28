<?hh

class Foo {
  public function bar() :mixed{
    $x = function() {
      var_dump(static::class);
      var_dump(self::class);
    };
    $x();
  }
}

class Herp extends Foo {
}


<<__EntryPoint>>
function main_late_static_binding_keywords() :mixed{
(new Herp())->bar();
}
