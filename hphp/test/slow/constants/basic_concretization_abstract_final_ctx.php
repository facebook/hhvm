<?hh

abstract class A {
  abstract const ctx C = [];
}
abstract final class AFinal extends A {}

<<__EntryPoint>>
function main()[AFinal::C]: void {
  echo "context should be concrete\n";
}
