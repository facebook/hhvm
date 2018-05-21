<?hh

<<__Sealed(SomeTrait::class)>>
trait MyTrait { public static function foo(): int {return 1;} }

var_dump(SomeTrait2::foo());

function __autoload($x) {
  require_once "sealed_classes11.inc";
}
