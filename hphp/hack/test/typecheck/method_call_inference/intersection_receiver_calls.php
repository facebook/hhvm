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

class Test {
  public static function test_concrete_1(C $c) : void {
    $c as A;
    hh_show($c); // C & A
    $b = $c->x_to_y();
    hh_show($b); // B, as C does not implement x_to_y
  }

  public static function test_concrete_2(D $d) : void {
    $d as A;
    hh_show($d); // D & A
    $b_and_c = $d->x_to_y();
    hh_show($b_and_c); // B & C, as both A and D implement x_to_y
  }

  public static function test_concrete_nullsafe_1(?C $c) : void {
    $c as ?A;
    hh_show($c); // ?(C & A)
    $b = $c?->x_to_y();
    hh_show($b); // ?B, as C does not implement x_to_y
  }

  public static function test_concrete_nullsafe_2(?D $d) : void {
    $d as ?A;
    hh_show($d); // ?(D & A)
    $b_and_c = $d?->x_to_y();
    hh_show($b_and_c); // ?(B & C), as both A and D implement x_to_y
  }

  public static function test_concrete_nullsafe_3(?D $d) : void {
    $d as ?A;
    hh_show($d); // ?(D & A)
    $b_and_c = $d->x_to_y(); // Error - $d can be null
  }

  // TODO: tests for intersections with unresolved types, when fixed
}

class TestDynamic {
  public static function test_concrete(dynamic $dyn) : void {
    $dyn as A;
    $dyn_and_b = $dyn->x_to_y();
    hh_show($dyn_and_b); // dynamic & B
  }

  public static function test_nullsafe_concrete(dynamic $dyn) : void {
    $dyn as ?A;
    $dyn_and_b = $dyn?->x_to_y();
    hh_show($dyn_and_b); // dynamic & ?B
  }

  // TODO: tests for intersections with unresolved types, when fixed
}
