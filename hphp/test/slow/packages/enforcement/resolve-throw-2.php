<?hh

module a.b;

<<__EntryPoint>>
function main_resolve_throw_2() {
  try {
    $f = bar1();
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    $f = bar2();
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    $f = bar3();
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }
}
