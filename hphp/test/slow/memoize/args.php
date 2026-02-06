<?hh
class A {

  private static $oneArgMethI = 10;
  <<__Memoize>>
  public function oneArgMeth(int $a): int {
    $__lval_tmp_0 = self::$oneArgMethI;
    self::$oneArgMethI++;
    return $a + $__lval_tmp_0;
  }

  private static $multiArgMethI = 20;
  <<__Memoize>>
  public function multiArgMeth(int $a, int $b, int $c): int {
    $__lval_tmp_1 = self::$multiArgMethI;
    self::$multiArgMethI++;
    return ($a * $b * $c) + $__lval_tmp_1;
  }

  private static $oneArgStaticI = 30;
  <<__Memoize>>
  public static function oneArgStatic(int $a): int {
    $__lval_tmp_2 = self::$oneArgStaticI;
    self::$oneArgStaticI++;
    return $a + $__lval_tmp_2;
  }

  private static $multiArgStaticI = 40;
  <<__Memoize>>
  public static function multiArgStatic(int $a, int $b, int $c): int {
    $__lval_tmp_3 = self::$multiArgStaticI;
    self::$multiArgStaticI++;
    return ($a * $b * $c) + $__lval_tmp_3;
  }
}

abstract final class OneargtoplevelStatics {
  public static $i = 50;
}

<<__Memoize>>
function oneArgTopLevel(int $a): int {
  $__lval_tmp_4 = OneargtoplevelStatics::$i;
  OneargtoplevelStatics::$i++;
  return $a + $__lval_tmp_4;
}

abstract final class MultiargtoplevelStatics {
  public static $i = 60;
}
<<__Memoize>>
function multiArgTopLevel(int $a, int $b, int $c): int {
  $__lval_tmp_5 = MultiargtoplevelStatics::$i;
  MultiargtoplevelStatics::$i++;
  return ($a * $b * $c) + $__lval_tmp_5;
}

abstract final class NullintfnStatics {
  public static $i = 70;
}

<<__Memoize>>
function nullIntFn(?int $a): int {
  $__lval_tmp_6 = NullintfnStatics::$i;
  NullintfnStatics::$i++;
  return (int)$a + $__lval_tmp_6;
}

abstract final class BoolfnStatics {
  public static $i = 80;
}
<<__Memoize>>
function boolFn(bool $a): int {
  $__lval_tmp_7 = BoolfnStatics::$i;
  BoolfnStatics::$i++;
  return ($a ? 42 : -42) + $__lval_tmp_7;
}

abstract final class NullboolfnStatics {
  public static $i = 90;
}
<<__Memoize>>
function nullBoolFn(?bool $a): int {
  $__lval_tmp_8 = NullboolfnStatics::$i;
  NullboolfnStatics::$i++;
  return ($a ? 42 : -42) + $__lval_tmp_8;
}

abstract final class StringfnStatics {
  public static $s = '';
}
<<__Memoize>>
function stringFn(string $a): string {StringfnStatics::$s = StringfnStatics::$s.StringfnStatics::$s.$a; return StringfnStatics::$s;}

abstract final class NullretStatics {
  public static $i = 100;
}

<<__Memoize>>
function nullRet(int $a): ?int {
  if ($a > 10) {
    return null;
  }
  $__lval_tmp_9 = NullretStatics::$i;
  NullretStatics::$i++;
  return $a + $__lval_tmp_9;
}

abstract final class NorettypeStatics {
  public static $i = 110;
}
<<__Memoize>>
function noRetType(int $a, int $b): ?int {
  if ($a * $b > 10) {
    return null;
  }
  $__lval_tmp_10 = NorettypeStatics::$i;
  NorettypeStatics::$i++;
  return ($a * $b) + $__lval_tmp_10;
}

abstract final class DefaultargsStatics {
  public static $i = 120;
}

<<__Memoize>>
function defaultArgs(int $a, int $b = 5) :mixed{
  $__lval_tmp_11 = DefaultargsStatics::$i;
  DefaultargsStatics::$i++;
  return ($a * $b) + $__lval_tmp_11;
}

abstract final class GenericsStatics {
  public static $i = 130;
}

<<__Memoize>>
function generics<T>(T $a, T $b): T {
  $__lval_tmp_12 = GenericsStatics::$i;
  GenericsStatics::$i++;
  return $__lval_tmp_12 % 2 ? $a : $b;
}

abstract final class BoolnoretStatics {
  public static $i = 140;
}

<<__Memoize>>
function boolNoRet(bool $a) :mixed{
  $__lval_tmp_13 = BoolnoretStatics::$i;
  BoolnoretStatics::$i++;
  return ($a ? 42 : -42) + $__lval_tmp_13;
}

interface I extends HH\IMemoizeParam {}

class O implements I {
  public function __construct(public string $a)[] {}
  public function getInstanceKey(): string { return $this->a; }
}

abstract final class MemoizeobjStatics {
  public static $i = 150;
}

<<__Memoize>>
function memoizeObj(I $obj) :mixed{
  $__lval_tmp_14 = MemoizeobjStatics::$i;
  MemoizeobjStatics::$i++;
  return $__lval_tmp_14;
}

<<__EntryPoint>>
function main_args() :mixed{
echo "Test each kind of function call with one and many args\n";
$a = new A();
echo $a->oneArgMeth(1).' ';
echo $a->oneArgMeth(1).' ';
echo $a->oneArgMeth(2).' ';
echo $a->oneArgMeth(2)."\n";

echo $a->multiArgMeth(1, 2, 3).' ';
echo $a->multiArgMeth(1, 2, 3).' ';
echo $a->multiArgMeth(4, 5, 6).' ';
echo $a->multiArgMeth(4, 5, 6)."\n";

echo A::oneArgStatic(1).' ';
echo A::oneArgStatic(1).' ';
echo A::oneArgStatic(2).' ';
echo A::oneArgStatic(2)."\n";

echo A::multiArgStatic(1, 2, 3).' ';
echo A::multiArgStatic(1, 2, 3).' ';
echo A::multiArgStatic(4, 5, 6).' ';
echo A::multiArgStatic(4, 5, 6)."\n";

echo oneArgTopLevel(1).' ';
echo oneArgTopLevel(1).' ';
echo oneArgTopLevel(2).' ';
echo oneArgTopLevel(2)."\n";

echo multiArgTopLevel(1, 2, 3).' ';
echo multiArgTopLevel(1, 2, 3).' ';
echo multiArgTopLevel(4, 5, 6).' ';
echo multiArgTopLevel(4, 5, 6)."\n";

echo "Test that arg order matters in caching\n";
echo multiArgTopLevel(3, 1, 2).' ';
echo multiArgTopLevel(3, 1, 2).' ';
echo multiArgTopLevel(2, 3, 1).' ';
echo multiArgTopLevel(2, 3, 1)."\n";

echo "Test each of the types currently supported\n";
echo nullIntFn(0).' ';
echo nullIntFn(0).' ';
echo nullIntFn(null).' ';
echo nullIntFn(null)."\n";

echo boolFn(false).' ';
echo boolFn(false).' ';
echo boolFn(true).' ';
echo boolFn(true)."\n";

echo nullBoolFn(false).' ';
echo nullBoolFn(false).' ';
echo nullBoolFn(null).' ';
echo nullBoolFn(null)."\n";

echo stringFn('a').' ';
echo stringFn('a').' ';
echo stringFn('b').' ';
echo stringFn('b').' ';
echo stringFn('').' ';
echo stringFn('')."\n";

echo "Test nullable/unspecified return types\n";
echo (nullRet(1) ?: 'null').' ';
echo (nullRet(1) ?: 'null').' ';
echo (nullRet(20) ?: 'null').' ';
echo (nullRet(20) ?: 'null')."\n";

echo (noRetType(1,2) ?: 'null').' ';
echo (noRetType(1,2) ?: 'null').' ';
echo (noRetType(4,5) ?: 'null').' ';
echo (noRetType(4,5) ?: 'null')."\n";

echo "Test default args\n";
echo defaultArgs(1).' ';
echo defaultArgs(1,5).' ';
echo defaultArgs(1,5).' ';
echo defaultArgs(2,6).' ';
echo defaultArgs(2,6)."\n";

echo "Test generics\n";
echo generics(1,2).' ';
echo generics(1,2).' ';
echo generics('a','b').' ';
echo generics('a','b')."\n";

echo "Test bool without return type (test for issue in #5307125)\n";
echo boolNoRet(true).' ';
echo boolNoRet(true).' ';
echo boolNoRet(false).' ';
echo boolNoRet(false)."\n";

echo "Test objects\n";
echo memoizeObj(new O('a')).' ';
echo memoizeObj(new O('a')).' ';
echo memoizeObj(new O('b')).' ';
echo memoizeObj(new O('b')).' ';
}
