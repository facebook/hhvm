<?hh

module a.b;

<<__EntryPoint>>
function main_resolve_throw_1() {
  try {
    baz1();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $f = baz2();
  try {
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  $f = baz3();
  try {
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }
}
