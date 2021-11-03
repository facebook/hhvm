<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C<T> {
  public function __construct(private T $x) {}
  public function get() : T { return $this->x; }
}

<<__SupportDynamicType>>
class D<T> {
  public function __construct(private T $x) {}
  public function set(T $x) : void { $this->x = $x; }
}

function f() : void {
  new C<mixed>(1) upcast dynamic;
}
