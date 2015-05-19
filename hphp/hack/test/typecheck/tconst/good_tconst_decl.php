<?hh
abstract class A {
  abstract const type abs_class_by_class as arraykey;

  final public function __construct(private this::abs_class_by_class $id) {}

  public function overriding(): Aint::vec {
    return Vector {};
  }

  public function getID(): this::abs_class_by_class {
    return $this->id;
  }

  public function testA(Aint::me::me::me::Aint $x): void {}
}

class Aint extends A {
  const type abs_class_by_class = int;
  const type me as Aint = Aint;
  const type Aint = self::hop;
  const type hop = int;
  const type vec = ConstVector<int>;

  public function test(): void {
    $this->testA(101);
  }

  public function overriding(): ImmVector<int> {
    return ImmVector {};
  }
}

class Astr extends A {
  const type abs_class_by_class = string;
}

abstract class B {
  abstract const type AType as A;
  abstract public static function getAID(): this::AType::abs_class_by_class;
}

class Bint extends B {
  const type AType = Aint;
  public static function getAID(): this::AType::abs_class_by_class {
    return 0;
  }
}

class Bstr extends B {
  const type AType = Astr;
  public static function getAID(): this::AType::abs_class_by_class {
    return '';
  }
}

function test(Aint $x): int {
  return $x->getID();
}
