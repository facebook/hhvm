<?hh

<<file: __EnableUnstableFeatures('readonly')>>
class Foo {
  public static readonly vec<vec<vec<int>>> $c =
    vec[vec[vec[0], vec[1], vec[2]]];
  public static readonly vec<Map<int, Map<int, int>>> $bad_c = vec[Map{1 => Map{1 => 2}}];
  public static readonly Map<int, Map<int, int>> $bad_c1 = Map{1 => Map{1 => 2}};
}

<<__EntryPoint>>
function test(): void {
  unset(Foo::$c[0][0][0]);
  try {
    unset(Foo::$bad_c[0][1][1]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  unset(Foo::$bad_c1[1][1]);
}
