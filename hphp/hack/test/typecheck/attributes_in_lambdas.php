<?hh // strict

function f(): void {
  $a = <<__Rx>> () ==> 1;
  $b = <<__Rx>> async {
    return 1;
  };
  $c = <<__Rx>> function() {
    return 1;
  };
  $d = <<__Rx>> function(): int use ($c) {
    return 1;
  };
  $f = <<__Rx>> $x ==> 1;
  $g = <<__Rx>> coroutine () ==> {
    return 1;
  };
}
