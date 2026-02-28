<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(int $i): void {

  switch ($i) {
    case 0: {
      let $y: int; // error
      let $a: int; // error
      switch ($i) {
        case 0:
          $x1 = 1;
          break;
        case 1:
          let $x1: int; // error
          break;
        case 2:
          $x1 = 2;
          let $x2:int; // error
          break;
        default:
          break;
      }
      let $y: int; // error
      break;
    }
    default:
      let $a:int;
      let $y:int;
      let $x1:int;
      $x2 = 2;
  }
}
