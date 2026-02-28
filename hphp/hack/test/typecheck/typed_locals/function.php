<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  let $x: int;
}

function g(): void {
  let $x: int;
}

class C {
  public function f(): void {
    let $x: int;
  }

  public function g(): void {
    let $x: int;
  }
}
