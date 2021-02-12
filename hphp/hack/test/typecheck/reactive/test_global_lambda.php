<?hh // strict
class GlobalClassName {
  public static int $x = 0;

}


function fooLambda(): void {
  $x = <<__NonRx>>() ==> {
    return GlobalClassName::$x + 1;
  };
}
