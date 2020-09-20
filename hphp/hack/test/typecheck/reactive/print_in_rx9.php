<?hh // strict

<<__Rx>>
function f(): void {
  $a = <<__NonRx>>() ==> {
    print 1;
  };
  // ERROR
  $a();
}
