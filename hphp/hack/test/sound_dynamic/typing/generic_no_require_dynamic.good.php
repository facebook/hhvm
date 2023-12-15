<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C<T as supportdyn<mixed>> {
  public function __construct(private ~T $x) {}
  public function get() : ~T { return $this->x; }

  public function k((function (~T) : T) $f) : T {
    return $f($this->x);
  }
}

function f() : void {
  new C<int>(1) upcast dynamic;
}
