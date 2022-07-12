<?hh

abstract class A {
  abstract const ctx C = [];
}
abstract class AAbstract extends A {}

<<__EntryPoint>>
function main()[AAbstract::C]: void {
  echo "this file should fatal TODO(T89365743)\n";
}
