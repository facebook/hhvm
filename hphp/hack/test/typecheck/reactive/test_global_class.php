<?hh // strict
class GlobalClassName {
  public static int $x = 0;

}


function foo()[rx]: void {
  $y = GlobalClassName::$x + 1;
}
