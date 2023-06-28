<?hh

abstract class C {
  abstract const type T;

  public static function isT(mixed $x): void {
    if ($x is this::T) {
      echo "T\n";
    } else {
      echo "not T\n";
    }
  }
}


<<__EntryPoint>>
function main_is_expression_tc_abstract() :mixed{
C::isT('foo');
}
