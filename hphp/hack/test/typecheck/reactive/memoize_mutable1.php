<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "";
  }
}

<<__Rx, __Memoize>>
function f(<<__Mutable>> A $a): void {
}
