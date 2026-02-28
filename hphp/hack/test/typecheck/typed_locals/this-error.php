<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

class C {
  public function f(): void {
    let $this:int = 4;
  }
}
