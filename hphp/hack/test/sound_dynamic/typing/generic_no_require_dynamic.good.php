<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C<T> {
  public function __construct(private T $x) {}
  public function get() : T { return $this->x; }
}

function f() : void {
  new C<int>(1) upcast dynamic;
}
