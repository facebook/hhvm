<?hh

class A {
  public static function foo() :mixed{
    return static::$bar;
  }
}

class B extends A {
  public static $bar = 42;
}

class C extends A {
  public static $bar = 43;
}

function main() :mixed{
  return B::foo() + C::foo();
}


<<__EntryPoint>>
function main_inline_lateboundcls() :mixed{
echo var_dump(main());
}
