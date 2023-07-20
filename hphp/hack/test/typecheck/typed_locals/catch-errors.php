<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(bool $b): void {
  let $x1:int = 1;
  let $a:int = 1;
  try {
   let $x2:int = 1;
   let $x3:int = 1;
   if ($b) {
      throw new Exception();
    }
  }
  catch (Exception $z) {
    $x1 = ""; // error
    $x2 = ""; // error
    let $a:int; // error
    let $x3:int; // error
    let $y:int = 1;
    let $y:int;  // error
    let $z:Exception; // error
    let $c:int;
  }
  let $c:int; // error
}
