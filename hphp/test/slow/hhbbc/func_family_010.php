<?hh

class B {
  public static function go() :mixed{
    $x = static::make("foo");
    $x->heh();
    return $x;
  }

  public static function make() :mixed{ throw new Exception('a'); }
}

class C {
  function __construct(private string $x)[] {}
  function heh() :mixed{ echo $this->x; }
}

class D1 extends B {
  static function make(string $x) :mixed{
    return new C($x);
  }
}

class D2 extends B {
  static function make(string $x) :mixed{
    return new C($x);
  }
}


<<__EntryPoint>>
function main_func_family_010() :mixed{
D1::go();
D2::go();
}
