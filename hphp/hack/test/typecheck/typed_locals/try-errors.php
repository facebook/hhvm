<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(bool $b): void {
  let $x:int = 1;
  let $z:int = 1;
  try {
    $x = ""; // error
    let $x:int; // error
    let $y:int = 1;
    let $y:int;  // error
    if ($b) {
      throw new Exception();
    }
  }
  catch (Exception $z) {} // error
}
