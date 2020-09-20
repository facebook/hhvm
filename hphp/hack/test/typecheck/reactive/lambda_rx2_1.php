<?hh // partial

<<__RxShallow>>
function g(): int {
  return 1;
}

<<__Rx>>
function f(): void {
  // OK: lambda is shallow
  $a = <<__RxShallow>>() ==> {
    g();
  };
}
