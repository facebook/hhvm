<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

class C extends Exception {}

function f(bool $b): void {
  let $x:int = 1;
  let $e:Exception;
  try {
    $x;
    $x = 1;
    let $y:string = "";
    if ($b) {
      throw new C();
    }
  }
  catch (C $e) {
    $x = 1;
    $y = "";
    let $z:bool = false;
  }
  finally {
    $x = 1;
    $y = "";
    $z = true;
    let $w:float = 3.3;
  }
  $x = 1;
  $w;
}
