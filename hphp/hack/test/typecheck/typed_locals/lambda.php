<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(bool $b): void {
  let $x: int;

  if (
    (
      $a ==> {
        $x = 1;
        let $y: string = "";
        $y = "1";
        let $z: string = "";
        return $a;
      }
    )($b)
  ) {
    let $y: int = 1;
  }
    let $z: int = 1;
}
