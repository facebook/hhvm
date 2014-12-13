<?hh

abstract class A {
  abstract public type const abs_class_by_class = arraykey;

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
  public type const abs_class_by_class = int;
  public type const me = Aint;
  public type const Aint = self::hop;
  public type const hop = int;
  public type const vec = ConstVector<int>;

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
  public type const abs_class_by_class = string;
  public function __construct(
    string $id,
  ) {
    $this->id = $id;
  }
}

abstract class B {
  abstract public type const AType = A;
  abstract public static function getAID(): static::AType::abs_class_by_class;
}

class Bint extends B {
  public type const AType = Aint;
  public static function getAID(): static::AType::abs_class_by_class {
    return 0;
  }
}

class Bstr extends B {
  public type const AType = Astr;
  public static function getAID(): static::AType::abs_class_by_class {
    return '';
  }
}

function test(Aint $x): int {
  return $x->getID();
}
