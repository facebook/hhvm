<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(int $i): void {

  let $y: int;
  switch ($i) {
    case 0:
      let $x0: int;
      $x1 = 1;
      $x2 = 2;
      let $x3: int;
      break;
    case 1:
      $x0 = 1;
      let $x1: int;
      $x2 = 2;
      $x3 = 3;
      break;
    case 2:
      $x0 = 1;
      $x1 = 2;
      let $x2: int;
      let $x3: int;
      break;
    default:
      break;
  }
  let $y: int;
}
