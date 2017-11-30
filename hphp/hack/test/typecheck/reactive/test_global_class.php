<?hh // strict

class GlobalClassName {
  public static int $x = 0;

}

<<__Rx>>
function foo(): void {
  $y = GlobalClassName::$x + 1;
}
