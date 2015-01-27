<?hh
class A {
  <<__Memoize>>
  public function oneArgMeth(int $a): int {
    static $i = 10;
    return $a + $i++;
  }
  <<__Memoize>>
  public function multiArgMeth(int $a, int $b, int $c): int {
    static $i = 20;
    return ($a * $b * $c) + $i++;
  }
  <<__Memoize>>
  public static function oneArgStatic(int $a): int {
    static $i = 30;
    return $a + $i++;
  }
  <<__Memoize>>
  public static function multiArgStatic(int $a, int $b, int $c): int {
    static $i = 40;
    return ($a * $b * $c) + $i++;
  }
}

<<__Memoize>>
function oneArgTopLevel(int $a): int {static $i = 50; return $a + $i++;}
<<__Memoize>>
function multiArgTopLevel(int $a, int $b, int $c): int {
  static $i = 60;
  return ($a * $b * $c) + $i++;
}

<<__Memoize>>
function nullIntFn(?int $a): int {static $i = 70; return (int)$a + $i++;}
<<__Memoize>>
function boolFn(bool $a): int {static $i = 80; return ($a ? 42 : -42) + $i++;}
<<__Memoize>>
function nullBoolFn(?bool $a): int {
  static $i = 90;
  return ($a ? 42 : -42) + $i++;
}
<<__Memoize>>
function stringFn(string $a): string {static $s = ''; $s = $s.$s.$a; return $s;}

<<__Memoize>>
function nullRet(int $a): ?int {
  static $i = 100;
  if ($a > 10) {
    return null;
  }
  return $a + $i++;
}
<<__Memoize>>
function noRetType(int $a, int $b): ?int {
  static $i = 110;
  if ($a * $b > 10) {
    return null;
  }
  return ($a * $b) + $i++;
}

<<__Memoize>>
function defaultArgs(int $a, int $b = 5) {
  static $i = 120;
  return ($a * $b) + $i++;
}

<<__Memoize>>
function generics<T>(T $a, T $b): T {
  static $i = 130;
  return $i++ % 2 ? $a : $b;
}

<<__Memoize>>
function boolNoRet(bool $a) {static $i = 140; return ($a ? 42 : -42) + $i++;}

interface I extends HH\IMemoizeParam {}

class O implements I {
  public function __construct(public string $a) {}
  public function getInstanceKey(): string { return $this->a; }
}

<<__Memoize>>
function memoizeObj(I $obj) { static $i = 150; return $i++; }

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
