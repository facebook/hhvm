<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(): void {
  $a = <<__NonRx>>() ==> {
    print 1;
  };
  // ERROR
  $a();
}
