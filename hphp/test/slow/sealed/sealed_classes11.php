<?hh

<<__Sealed(SomeTrait::class)>>
trait MyTrait { public static function foo(): int {return 1;} }

function __autoload($x) {
  require_once "sealed_classes11.inc";
}


<<__EntryPoint>>
function main_sealed_classes11() {
var_dump(SomeTrait2::foo());
}
