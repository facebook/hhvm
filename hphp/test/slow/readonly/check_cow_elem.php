<?hh

<<file: __EnableUnstableFeatures('readonly')>>

class Foo {
  public static readonly vec<Vector<int>> $c = vec[Vector {1}];
  public static readonly vec<Vector<int>> $c1 = vec[Vector {1}];
  public static readonly vec<Map<arraykey, Map<arraykey, int>>> $c2 =
    vec[Map {'a' => Map {'b' => 5}}];
  public static readonly vec<Map<arraykey, int>> $c3 = vec[Map {'a' => 5}];
}

<<__EntryPoint>>
function test(): void {
  try {
    Foo::$c[0][0] = 5;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    Foo::$c1[0][0] = 5;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    Foo::$c2[0]['a']['b'] = 5;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    Foo::$c3[0]['a'] = 5;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
