<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function g(bool $b): void {
  let $x: int;

  if (
    (
      function($a) {
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

function h(bool $b): void {
  let $x: int = 1;

  if (
    (
      function($a) use ($x) {
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

function f(int $i): void {
  let $x:int = 1;
  (function ($y) { let $i:string = "1"; let $x:string = "2"; })(1);
}
