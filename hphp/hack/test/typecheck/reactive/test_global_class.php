<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class GlobalClassName {
  public static int $x = 0;

}

<<__Rx>>
function foo(): void {
  $y = GlobalClassName::$x + 1;
}
