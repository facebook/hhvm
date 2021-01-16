<?hh // strict
<<__Rx>>
function f()[rx]: void {
  $a = <<__NonRx>>()[defaults] ==> {
    print 1;
  };
  // ERROR
  $a();
}
