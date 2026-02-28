<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C {
  const int num = 1;
  public static function static_m(): void {}
  public function m(): void {}
}

function simple_upcast1(): void {
  $y = C::class upcast dynamic;
  $y::static_m();
}

function simple_upcast2(): void {
  $y = new C() upcast dynamic;
  $y->m();
}

function simple_upcast3(): void {
  $y = new C() upcast dynamic;
  $y::static_m();
}

function simple_upcast4(): void {
  $y = new C() upcast dynamic;
  $y->num;
}
