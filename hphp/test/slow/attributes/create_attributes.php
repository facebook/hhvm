<?hh

class C implements HH\ClassAttribute {}
class F implements HH\FunctionAttribute {}
class Met implements HH\MethodAttribute {}
class IProp implements HH\InstancePropertyAttribute {}
class SProp implements HH\StaticPropertyAttribute {}
class P implements HH\ParameterAttribute {}
class T implements HH\TypeAliasAttribute {}

<<F>>
function ff(<<P>>int $i): void {}

<<C>>
class CC {
  <<IProp>>
  private int $mem = 4;
  <<SProp>>
  private static int $smem = 42;
  <<Met>>
  private function met(): void {}
}

<<T>>
type tt = int;
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
