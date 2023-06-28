<?hh

module a.b;

<<__EntryPoint>>
function main_resolve_throw_2() :mixed{
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

  try {
    bar4();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    $f = bar5();
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    $c = cbar1();
    new $c;
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    $c = cbar2();
    $f();
  } catch (Exception $e) {
    echo pp_exn($e);
  }

  try {
    cbar3();
  } catch (Exception $e) {
    echo pp_exn($e);
  }
}
