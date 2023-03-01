<?hh

namespace SealedClass17;

<<__Sealed(SomeOtherClass::class)>>
enum SomeSealedEnum: int {}


<<__EntryPoint>>
function main(): void {
  echo "Done\n";
}
