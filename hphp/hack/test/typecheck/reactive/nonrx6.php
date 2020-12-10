<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(): void {
  $a = <<__NonRx("?", 1)>>function() {
    return 1;
  };
}
