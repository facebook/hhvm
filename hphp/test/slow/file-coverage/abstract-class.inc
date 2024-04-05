<?hh
abstract class JustAnAbstractClass {
  abstract public static function hello(): string;
  function isEnabled(): bool {
    return true;
  }
}

class ConcreteClass extends JustAnAbstractClass {
  public static function hello(): string {
    return "hi";
  }
}

function cover_abstract_class() {
  $c = new ConcreteClass();
  $c->isEnabled();
  $c::hello();
}
