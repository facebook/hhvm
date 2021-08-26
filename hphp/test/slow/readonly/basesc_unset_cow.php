<?hh

<<file: __EnableUnstableFeatures('readonly')>>
class Foo {
  public static readonly vec<vec<vec<int>>> $c =
    vec[vec[vec[0], vec[1], vec[2]]];
  public static readonly vec<Vector<vec<int>>> $bad_c =
    vec[Vector {vec[0], vec[1], vec[2]}];
  public static readonly Vector<vec<int>> $bad_c1 =
    Vector {vec[0], vec[1], vec[2]};
}

<<__EntryPoint>>
function test(): void {
  unset(Foo::$c[0][0][0]);
  try {
    unset(Foo::$bad_c[0][0][0]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  unset(Foo::$bad_c1[0]);
}
