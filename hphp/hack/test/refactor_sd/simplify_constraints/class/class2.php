<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C {
  public function m(): void {}
  public static function static_m(): void {}
}

<<__SupportDynamicType>>
class D {
  public function m(): void {}
}

function while_test(): void {
  $index = 1;
  while ($index < 10) {
    if ($index == 9) {
      (new C() upcast dynamic)->m();
      (C::class upcast dynamic)->m();
    }
    $index++;
  }
}

function if_test(): void {
  $b = true;
  $c = false;
  $x = new D();
  $y = D::class;
  if ($b) {
    if ($c) {
      $x = new D() upcast dynamic;
    } else {
      (new C() upcast dynamic)->m();
    }
  } else {
    if ($c) {
      $x = new C();
      $y = C::class;
      $y::static_m();
    } else {
      (C::class upcast dynamic)->m();
    }
  }
  ($x upcast dynamic)->m();
  ($y upcast dynamic)->m();
}
