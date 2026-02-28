<?hh

function make_tvar<T>(): T { // actually this does not work creating a tvar...
  throw new Exception();
}
function make_tvaropt<T>(): ?T { // ... but this does
  return null;
}

function test(bool $b): void {
  $x = ($b ? make_tvaropt() : 1);
  hh_show($x);

  $x = ($b ? make_tvaropt() : make_tvaropt());
  hh_show($x);

  if ($b) {
    $x = make_tvaropt();
  } else if ($b) {
    $x = null;
  } else if ($b) {
    $x = make_tvaropt();
  } else if ($b) {
    $x = null;
  } else if ($b) {
    $x = make_tvaropt();
  } else if ($b) {
    $x = null;
  } else if ($b) {
    $x = make_tvaropt();
  } else if ($b) {
    $x = null;
  } else if ($b) {
    $x = make_tvaropt();
  } else if ($b) {
    $x = null;
  } else if ($b) {
    $x = make_tvaropt();
  } else if ($b) {
    $x = null;
  } else if ($b) {
    $x = make_tvaropt();
  } else {
    $x = null;
  }
  hh_show($x); // no Toption(Tvar(Toption(Toption...)))
}
