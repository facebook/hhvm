<?hh // strict

<<file: Fi(dict['a' => 1, 'b' => 2])>>

class A {
  public function __construct(dict<string, int> $_) {}
}

class C extends A implements HH\ClassAttribute {}
class E extends A implements HH\EnumAttribute {}
class F extends A implements HH\FunctionAttribute {}
class Met extends A implements HH\MethodAttribute {}
class IProp extends A implements HH\InstancePropertyAttribute {}
class SProp extends A implements HH\StaticPropertyAttribute {}
class P extends A implements HH\ParameterAttribute {}
class TAlias extends A implements HH\TypeAliasAttribute {}
class Fi extends A implements HH\FileAttribute {}

<<F(dict['a' => 1, 'b' => 2])>>
function ff(<<P(dict['a' => 1, 'b' => 2])>>int $i): void {}

<<C(dict['a' => 1, 'b' => 2])>>
class CC {
  <<IProp(dict['a' => 1, 'b' => 2])>>
  private int $mem = 4;
  <<SProp(dict['a' => 1, 'b' => 2])>>
  private static int $smem = 42;
  <<Met(dict['a' => 1, 'b' => 2])>>
  private function met(): void {}
}

<<E(dict['a' => 1, 'b' => 2])>>
enum EE: int {}

<<TAlias(dict['a' => 1, 'b' => 2])>>
type tt = int;
