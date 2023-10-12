<?hh // strict

<<file: Fi>>

class C implements HH\ClassAttribute {}
class Cc implements HH\ClassConstantAttribute {}
class E implements HH\EnumAttribute {}
class F implements HH\FunctionAttribute {}
class Met implements HH\MethodAttribute {}
class IProp implements HH\InstancePropertyAttribute {}
class SProp implements HH\StaticPropertyAttribute {}
class P implements HH\ParameterAttribute {}
class T implements HH\TypeAliasAttribute {}
class Fi implements HH\FileAttribute {}
class Ec implements HH\EnumClassAttribute {}
class TParam implements HH\TypeParameterAttribute {}
class Tc implements HH\TypeConstantAttribute {}
class L implements HH\LambdaAttribute {}

<<F>>
function ff(<<P>>int $i): void {
  <<L>>
  () ==> {};
}

<<C>>
class CCC {
  <<Cc>>
  const int VAL = 64;
  <<IProp>>
  private int $mem = 4;
  <<SProp>>
  private static int $smem = 42;
  <<Tc>>
  const type Tu = string;
  <<Met>>
  private function met(): void {}
}

class CT<<<TParam>> reify T1> { }

<<E>>
enum EE: int {}

<<T>>
type tt = int;

<<Ec>>
enum class Ecc : EE {}
