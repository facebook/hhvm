<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

class C implements IDisposable {
  public function __dispose(): void {}
}

function f(): void {
  let $a:C;
  using ($a = new C(), $b = new C()) {
  }
}
