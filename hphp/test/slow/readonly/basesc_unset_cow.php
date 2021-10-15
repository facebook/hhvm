<?hh

<<file: __EnableUnstableFeatures('readonly')>>
class Foo {
  public static readonly vec<vec<vec<int>>> $c =
    vec[vec[vec[0], vec[1], vec[2]]];
  public static readonly vec<Map<int, Map<int, int>>> $bad_c = vec[Map{1 => Map{1 => 2}}];
  public static readonly vec<Map<int, int>> $bad_c1 = vec[Map{1 => 2}];
  public static readonly Map<int, Map<int, int>> $bad_c2 = Map{1 => Map{1 => 2}};
  public static readonly Map<int, int> $bad_c3 = Map{1 => 2};
}

<<__EntryPoint>>
function test(): void {
  unset(Foo::$c[0][0][0]);
  try {
    unset(Foo::$bad_c[0][1][1]); // [0] must be COW. [1] must be COW.
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    unset(Foo::$bad_c1[0][1]); // [0] must be COW.
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    unset(Foo::$bad_c2[1][1]); // bad_c2 must be COW.
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  unset(Foo::$bad_c3[1]); // bad_c3 must be COW.
}
