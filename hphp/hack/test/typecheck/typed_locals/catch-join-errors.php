<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(bool $b): void {
  let $a:int;
  try {
   if ($b) {
      throw new Exception();
    }
  }
  catch (Exception $z) {
    let $x:int; // error
  }
  catch (Throwable $z) {
    let $x:int;
  }
  let $a:int; // error
}
