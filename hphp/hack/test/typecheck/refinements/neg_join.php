<?hh

function f(float $f) : void {}
function nn(nonnull $f) : void {}

function example(mixed $m): void {
  if ($m is num) {
    if ($m is int) {

    }
    else {
      f($m);
    }
  } else if ($m is bool) {
  } else if ($m is string) {
  } else if ($m is null) {
        hh_show($m);


  } else {
    nn($m);
    hh_show($m);

  }
  hh_show($m);

}
