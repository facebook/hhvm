<?hh


class A {
  private $a = 1;
  public function __sleep() :mixed{
    return SerializationPrivateProperties::$g;
  }
  public function seta($a) :mixed{
    $this->a = $a;
  }
}
class B extends A {
  public function __sleep() :mixed{
    return SerializationPrivateProperties::$g;
  }

  static function test($a, $elems, $p = null) :mixed{

    $a->seta(42);
    SerializationPrivateProperties::$g = $elems;
    $s = serialize($a);
    var_export($s);
    echo "\n";
    $u = unserialize($s);
    var_dump($u);
    if ($p) {
      try {
        var_dump($u->$p);
      } catch (UndefinedPropertyException $e) {
        var_dump($e->getMessage());
      }
    }
  }
}

abstract final class SerializationPrivateProperties {
  public static $g;
}
<<__EntryPoint>>
function entrypoint_private_properties(): void {
  error_reporting(-1);
  B::test(new A, vec["a"]);
  B::test(new A, vec["\0A\0a"]);
  B::test(new A, vec["\0*\0a"]);
  B::test(new A, vec["\0*\0b"], "b");
  B::test(new A, vec["\0B\0b"], "b");
  B::test(new A, "foo");
  B::test(new B, vec["a"]);
  B::test(new B, vec["\0A\0a"]);
  B::test(new B, vec["\0*\0a"]);
  B::test(new B, vec["\0*\0b"], "b");
  B::test(new B, vec["\0B\0b"], "b");
}
