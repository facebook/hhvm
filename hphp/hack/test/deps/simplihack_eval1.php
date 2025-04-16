//// prompt.php
<?hh

function prompt(): string {
  return hello() . A::prompt();
}

//// a.php
<?hh

class A {
  public static function prompt (): string {
    return "world";
  }
}

//// b.php
<?hh

function hello(): string {
  return "hello";
}

//// use.php
<?hh
<<file:__EnableUnstableFeatures('simpli_hack')>>

<<__SimpliHack(prompt())>>
class Clazz {

}

class Method {
  <<__SimpliHack(prompt())>>
  public function f(): void {
  }
}

<<__SimpliHack(prompt())>>
function func(): void {
}

class ClassField {
  <<__SimpliHack(prompt())>>
  public int $f = 1;
}

<<__SimpliHack(prompt())>>
type TypeAlias = int;

class ClassConstant {
  <<__SimpliHack(prompt())>>
  const int F = 1;
}

class FunctionParam {
  public static function f(
    <<__SimpliHack(prompt())>> int $x,
  ): void {}
}

function functionParam(
  <<__SimpliHack(prompt())>> int $x,
): void {}

class ClassGeneric<<<__SimpliHack(prompt())>> T> {
}

class TypeConstant {
  <<__SimpliHack(prompt())>>
  const type T = int;
}

//// module.php
<?hh

<<__SimpliHack(prompt())>>
new module Mod {

}
