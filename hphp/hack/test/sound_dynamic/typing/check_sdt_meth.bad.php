<?hh

<<__SupportDynamicType>>
class D {
  public function f(vec<int> $x) : vec<int> { return $x; } }

<<__SupportDynamicType>>
class C {
  public function g(vec<int> $v) : vec<int> {
    if ($v is vec<_>) {
      return vec[4];
    }
    else {
      // $v : nothing in first check and $v : dynamic & not vec in second
      return (new D())->f($v, 4);
    }
  }
}
