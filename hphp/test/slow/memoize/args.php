<?hh
class A {

  private static $oneArgMethI = 10;
  <<__Memoize>>
  public function oneArgMeth(int $a): int {
    return $a + self::$oneArgMethI++;
  }

  private static $multiArgMethI = 20;
  <<__Memoize>>
  public function multiArgMeth(int $a, int $b, int $c): int {
    return ($a * $b * $c) + self::$multiArgMethI++;
  }

  private static $oneArgStaticI = 30;
  <<__Memoize>>
  public static function oneArgStatic(int $a): int {
    return $a + self::$oneArgStaticI++;
  }

  private static $multiArgStaticI = 40;
  <<__Memoize>>
  public static function multiArgStatic(int $a, int $b, int $c): int {
    return ($a * $b * $c) + self::$multiArgStaticI++;
  }
}

abstract final class OneargtoplevelStatics {
  public static $i = 50;
}

<<__Memoize>>
function oneArgTopLevel(int $a): int {return $a + OneargtoplevelStatics::$i++;}

abstract final class MultiargtoplevelStatics {
  public static $i = 60;
}
<<__Memoize>>
function multiArgTopLevel(int $a, int $b, int $c): int {
  return ($a * $b * $c) + MultiargtoplevelStatics::$i++;
}

abstract final class NullintfnStatics {
  public static $i = 70;
}

<<__Memoize>>
function nullIntFn(?int $a): int {return (int)$a + NullintfnStatics::$i++;}

abstract final class BoolfnStatics {
  public static $i = 80;
}
<<__Memoize>>
function boolFn(bool $a): int {return ($a ? 42 : -42) + BoolfnStatics::$i++;}

abstract final class NullboolfnStatics {
  public static $i = 90;
}
<<__Memoize>>
function nullBoolFn(?bool $a): int {
  return ($a ? 42 : -42) + NullboolfnStatics::$i++;
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
  return $a + NullretStatics::$i++;
}

abstract final class NorettypeStatics {
  public static $i = 110;
}
<<__Memoize>>
function noRetType(int $a, int $b): ?int {
  if ($a * $b > 10) {
    return null;
  }
  return ($a * $b) + NorettypeStatics::$i++;
}

abstract final class DefaultargsStatics {
  public static $i = 120;
}

<<__Memoize>>
function defaultArgs(int $a, int $b = 5) :mixed{
  return ($a * $b) + DefaultargsStatics::$i++;
}

abstract final class GenericsStatics {
  public static $i = 130;
}

<<__Memoize>>
function generics<T>(T $a, T $b): T {
  return GenericsStatics::$i++ % 2 ? $a : $b;
}

abstract final class BoolnoretStatics {
  public static $i = 140;
}

<<__Memoize>>
function boolNoRet(bool $a) :mixed{return ($a ? 42 : -42) + BoolnoretStatics::$i++;}

interface I extends HH\IMemoizeParam {}

class O implements I {
  public function __construct(public string $a)[] {}
  public function getInstanceKey(): string { return $this->a; }
}

abstract final class MemoizeobjStatics {
  public static $i = 150;
}

<<__Memoize>>
function memoizeObj(I $obj) :mixed{ return MemoizeobjStatics::$i++; }


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
