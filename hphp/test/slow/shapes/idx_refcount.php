<?hh

class C {}

<<__EntryPoint>>
function main_idx() :mixed{
  // Make sure we can't constant-fold this expression to get a static string.
  $refcounted_string = __hhvm_intrinsics\launder_value('xyz')[0];

  // In this test, we call Shapes::idx with refcounted and uncounted results
  // and default values, to make sure that refcounting is working correctly.
  $shapes = vec[shape(), shape('x' => 4), shape('x' => new C())];
  $keys = vec[17, 'x', $refcounted_string];
  $defaults = vec[null, 42, new C()];

  foreach ($shapes as $shape) {
    foreach ($keys as $key) {
      var_dump(Shapes::idx($shape, $key));
      foreach ($defaults as $default) {
        var_dump(Shapes::idx($shape, $key, $default));
      }
    }
  }
}
