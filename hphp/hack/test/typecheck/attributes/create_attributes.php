<?hh // strict

class __Attribute__C implements HH\ClassAttribute {}
class __Attribute__E implements HH\EnumAttribute {}
class __Attribute__F implements HH\FunctionAttribute {}
class __Attribute__Met implements HH\MethodAttribute {}
class __Attribute__IProp implements HH\InstancePropertyAttribute {}
class __Attribute__SProp implements HH\StaticPropertyAttribute {}
class __Attribute__P implements HH\ParameterAttribute {}
class __Attribute__T implements HH\TypeAliasAttribute {}

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
type t = int;
