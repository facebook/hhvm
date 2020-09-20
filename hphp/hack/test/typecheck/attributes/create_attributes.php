<?hh // strict

<<file: Fi>>

class C implements HH\ClassAttribute {}
class E implements HH\EnumAttribute {}
class F implements HH\FunctionAttribute {}
class Met implements HH\MethodAttribute {}
class IProp implements HH\InstancePropertyAttribute {}
class SProp implements HH\StaticPropertyAttribute {}
class P implements HH\ParameterAttribute {}
class T implements HH\TypeAliasAttribute {}
class Fi implements HH\FileAttribute {}

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

<<E>>
enum EE: int {}

<<T>>
type tt = int;
