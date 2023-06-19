<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(int $e): void {
  switch ($e) {
    case 0:
      let $x: int = 1;
      // FALLTHROUGH
    default:
      $x = ""; // error
      break;
  }
}
