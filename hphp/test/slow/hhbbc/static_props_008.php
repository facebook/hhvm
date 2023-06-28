<?hh

class A { public function bummer($x) :mixed{var_dump($x);} }
class B { public function bummer(inout $x) :mixed{$x = "asd"; var_dump($x);} }

class Foob {
  private static $maybe_boxed = 0;
  private static $ok = "ok";

  public static function a($y) :mixed{
    $y->bummer(self::$maybe_boxed);
  }
  public static function b($y) :mixed{
    $maybe_boxed = self::$maybe_boxed;
    $y->bummer(inout $maybe_boxed);
    self::$maybe_boxed = $maybe_boxed;
  }
  public static function get() :mixed{ return self::$maybe_boxed; }
  public static function ok() :mixed{ return self::$ok; }
}

function ok() :mixed{ return Foob::ok(); }

<<__EntryPoint>>
function main() :mixed{
  var_dump(Foob::get());
  Foob::a(new A());
  var_dump(Foob::get());
  Foob::b(new B());
  var_dump(Foob::get());
  var_dump(Foob::ok());
  var_dump(ok());
}
