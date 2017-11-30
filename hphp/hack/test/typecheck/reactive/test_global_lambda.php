<?hh // strict

class GlobalClassName {
  public static int $x = 0;

}

<<__Rx>>
function fooLambda(): void {
  $x = () ==> {
    return GlobalClassName::$x + 1;
  };
}
