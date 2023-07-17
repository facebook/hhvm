<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(bool $b): void {
  let $x1: int = 1;
  let $x2: int = 2;

  if (
    (
      function ($a) use ($x1, $x2, $b) {
        $x1 = ""; // error
        let $x2:int; // error
        let $a:int; // error
        let $b:bool; // error
        let $c:int;
        $c = ""; // error
        let $c:int; // error
        return false;
      }
    )($b)
  ) {
  }
}
