<?hh

abstract class A {
  abstract const type abs_class_by_class as arraykey;

  public static::abs_class_by_class $id;

  public function overriding(): Aint::vec {
    return Vector {};
  }

  public function getID(): static::abs_class_by_class {
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

  public function __construct(
    int $id,
  ) {
    $this->id = $id;
  }

  public function test(): void {
    $this->testA(101);
  }

  public function overriding(): ImmVector<int> {
    return ImmVector {};
  }
}

class Astr extends A {
  const type abs_class_by_class = string;
  public function __construct(
    string $id,
  ) {
    $this->id = $id;
  }
}

abstract class B {
  abstract const type AType = A;
  abstract public static function getAID(): static::AType::abs_class_by_class;
}

class Bint extends B {
  const type AType = Aint;
  public static function getAID(): static::AType::abs_class_by_class {
    return 0;
  }
}

class Bstr extends B {
  const type AType = Astr;
  public static function getAID(): static::AType::abs_class_by_class {
    return '';
  }
}

function test(Aint $x): int {
  return $x->getID();
}
