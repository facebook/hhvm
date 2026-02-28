<?hh

namespace SealedClass11;

<<__Sealed(SomeTrait::class)>>
trait MyTrait { public static function foo(): int {return 1;} }

<<__EntryPoint>>
function main_sealed_classes11() :mixed{
  var_dump(SomeTrait2::foo());
}
