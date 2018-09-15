<?hh // strict
class Bar {

}
function foo() : void {
  $x = async {
    return new Bar();
  };
  return $x;

}
