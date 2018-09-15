<?hh // strict

<<__Rx>>
function f(): void {
  $a = () ==> {
    print 1;
  };
  // ERROR
  $a();
}
