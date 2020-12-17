<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f()[rx]: void {
  $a = <<__NonRx>>()[defaults] ==> {
    print 1;
  };
  // ERROR
  $a();
}
