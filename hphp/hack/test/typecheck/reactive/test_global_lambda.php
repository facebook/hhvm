<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class GlobalClassName {
  public static int $x = 0;

}

<<__Rx>>
function fooLambda(): void {
  $x = <<__NonRx>>() ==> {
    return GlobalClassName::$x + 1;
  };
}
