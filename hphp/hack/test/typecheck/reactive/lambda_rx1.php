<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(): void {
  // OK: lambda is rx, can call rx
  $a = () ==> {
    f();
  };
}
