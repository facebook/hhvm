<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

interface I {}
;

interface J extends I {}
;

function f(bool $b, J $j, I $i): void {
  let $x: arraykey = 1;
  if ($b) {
    let $x: int = 1;
    let $y: J = $j;
  } else {
    let $y: J = $j;
  }
}

function g(bool $b): void {
  if ($b) {
    if ($b) {
    } else {
      let $x: int = 1;
    }
  } else {
    if ($b) {
      $x = 1;
    } else {
      $x = 1;
    }
  }

  if ($b) {
    if ($b) {
      $y = 1;
    } else {
      $y = 1;
    }
  } else {
    if ($b) {
    } else {
      let $y: int = 1;
    }
  }
}

function h(bool $b): void {
  $x = 1;
  if ($b) {
    let $x: int;
  }
}

function i(bool $b): void {
  if ($b) {
    let $x: int;
  }
  $x = "";
}
