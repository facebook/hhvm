<?hh

class A {
  public function x_to_y() : B {
    return new B();
  }
}

class B {}

class C {}

class D {
  public function x_to_y() : C {
    return new C();
  }
}

class Box<T> {
  public function get() : T {
    throw new Exception();
  }
}

function expect<T>(T $_) : void {}
function expectInter<TA, TB>((TA & TB) $_) : void {}

class Test {
  public static function test_concrete_1(C $c) : void {
    $c as A;
    $b = $c->x_to_y();
    expect<B>($b); // C does not implement x_to_y
  }

  public static function test_concrete_2(D $d) : void {
    $d as A;
    $b_and_c = $d->x_to_y();
    expectInter<B, C>($b_and_c); // both A and D implement x_to_y
  }

  public static function test_concrete_nullsafe_1(?C $c) : void {
    $c as ?A;
    $b = $c?->x_to_y();
    expect<?B>($b); // C does not implement x_to_y
  }

  public static function test_concrete_nullsafe_2(?D $d) : void {
    $d as ?A;
    $b_and_c = $d?->x_to_y();
    expectInter<?B, ?C>($b_and_c); // both A and D implement x_to_y
  }

  public static function test_concrete_nullsafe_3(?D $d) : void {
    $d as ?A; // ?(D & A)
    $b_and_c = $d->x_to_y(); // Error - $d can be null
  }

  public static function test_unresolved() : void {
    $box = new Box();
    $item = $box->get(); // $item : [unresolved]
    $item as A; // $item : [unresolved] & A
    $result = $item->x_to_y(); // $result : [unresolved] & B, with new type var
    expectInter<B, _>($result);
  }

  public static function test_unresolved_nullsafe_1(bool $b) : void {
    $box = new Box();
    $item = $b ? $box->get() : null; // $item : ?[unresolved]
    $item as ?A; // $item : ?([unresolved] & A)
    $result = $item?->x_to_y(); // $res : ?[unresolved] & ?B, with new type var
    expectInter<?B, _>($result);
  }

  public static function test_unresolved_nullsafe_2(bool $b) : void {
    $box = new Box();
    $item = $box->get(); // $item : [unresolved]
    $item as ?A; // $item : [unresolved] & ?A
    $result = $item?->x_to_y(); // $res : [unresolved] & ?B, with new type var
    expectInter<?B, _>($result);
  }
}

class TestDynamic {
  public static function test_concrete(dynamic $dyn) : void {
    $dyn as A;
    $dyn_and_b = $dyn->x_to_y();
    expectInter<dynamic, B>($dyn_and_b);
  }

  public static function test_nullsafe_concrete(dynamic $dyn) : void {
    $dyn as ?A;
    $dyn_and_b = $dyn?->x_to_y();
    expectInter<dynamic, ?B>($dyn_and_b);
  }
}
