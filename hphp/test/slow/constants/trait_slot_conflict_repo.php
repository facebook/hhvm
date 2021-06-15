<?hh

interface IA {}

interface ITr {
  const type T = int;
}
trait Tr implements ITr {}

class C implements IA {
  use Tr;
  const type T = int;
}

<<__EntryPoint>>
function main(): void {
  echo "main";
}
