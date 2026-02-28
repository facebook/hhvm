<?hh
function printFunc($rf) {
  $rps = $rf->getParameters();
  foreach($rps as $rp) {
    var_dump($rp->getTypeText());
  }
  var_dump($rf->getReturnTypeText());
}
function printClass($rc) {
  $rms = $rc->getMethods();
  $meths = dict[];
  foreach($rms as $rm) {
    $meths[$rm->getName()] = $rm;
  }
  ksort(inout $meths);
  foreach($meths as $meth) {
    printFunc($meth);
  }
  $rps = $rc->getProperties();
  $props = dict[];
  foreach($rps as $rp) {
    $props[$rp->getName()] = $rp;
  }
  ksort(inout $props);
  foreach($props as $prop) {
    var_dump($prop->getTypeText());
  }
}
function f() {
}
function f1(int $t) {
}
function f2(<<__Soft>> string $s) {
}
function f3(?:xhp:hello $x) {
}
function f4(): noreturn {
  throw Exception('f4 doesn\'t return');
}
function f100(): int {
}
function f101(): string {
}
function f102(): Vector<:xhp:element> {
}
function f200((string, Template<A, B, ?C>, ?int, Vector<Map<C, B>>) $tuple): ?int {
}
function f201((function (<<__Soft>> int, Map<string, Map<int, Vector<string>>>):Vector<C>) $i): ClassA {
}
function f202(:xhp:html-element $html): Map<string, Vector<:xhp:html-element>> {
}
function f203((int, Vector<string>) $tupple): varray<ClassA> {
}
function f204((function (int): Vector<string>) $f): darray<string, ClassA> {
}
function f300<X, Y>(Y $y, ?float $d): X {
}
function f301<X, Y>((function (): Vector<Y>) $f): darray<string, X> {
}
function f302<X, Y>((Y, X, float, string) $f): ?Y {
}
class C {
  <<__Soft>> public int $a;
  public ?string $b;
  public Vector<C> $v;
  public (string, (function(?int, (string, string)):void)) $c;
  public function __construct(?int $i, (int, string) $c) {
}
  static public function m1() {
}
  public function m2(<<__Soft>> float $d) : void {
}
  static public function m3(Vector<Map<int, Vector<?string>>> $v, :xhp:html $x) : <<__Soft>> darray<int, ?Vector<string>> {
}
  public function m4((function(<<__Soft>> int, (string, string)): void) $v) : varray<Map<string, :xhp:html>> {
}
}
class CT<X, Y> {
  <<__Soft>> public int $a;
  public ?string $b;
  public Vector<X> $v;
  public (string, (function(?int, (X, Y)):void)) $c;
  public function __construct(?int $i, (int, X) $c) {
}
  static public function m1() {
}
  public function m2(<<__Soft>> Y $d) : void {
}
  static public function m3(Vector<Map<int, Vector<?X>>> $v, :xhp:html $x) : <<__Soft>> darray<Y, ?Vector<string>> {
}
  public function m4((function(<<__Soft>> int, (X, string)): void) $v) : varray<Map<Y, :xhp:html>> {
}
}
trait T {
  static public function m1() {
}
  public function m2(<<__Soft>> int $d) : void {
}
  static public function m3(Vector<Map<int, Vector<?C>>> $v, :xhp:html $x) : <<__Soft>> darray<C, ?Vector<string>> {
}
  public function m4((function(<<__Soft>> int, (float, string)): void) $v) : varray<Map<CT<int, string>, :xhp:html>> {
}
}
class TC {
  use T;
}
function ff(Vector<int> $i, ?string $s, <<__Soft>> C $c,
            AnyArray $a, arraykey $k, this $t) {
}

<<__EntryPoint>>
function main_1366() {
$rf = new ReflectionFunction('f');
printFunc($rf);
$rf = new ReflectionFunction('f1');
printFunc($rf);
$rf = new ReflectionFunction('f2');
printFunc($rf);
$rf = new ReflectionFunction('f3');
printFunc($rf);
$rf = new ReflectionFunction('f4');
printFunc($rf);
$rf = new ReflectionFunction('f100');
printFunc($rf);
$rf = new ReflectionFunction('f101');
printFunc($rf);
$rf = new ReflectionFunction('f102');
printFunc($rf);
$rf = new ReflectionFunction('f200');
printFunc($rf);
$rf = new ReflectionFunction('f201');
printFunc($rf);
$rf = new ReflectionFunction('f202');
printFunc($rf);
$rf = new ReflectionFunction('f203');
printFunc($rf);
$rf = new ReflectionFunction('f204');
printFunc($rf);
$rf = new ReflectionFunction('f300');
printFunc($rf);
$rf = new ReflectionFunction('f301');
printFunc($rf);
$rf = new ReflectionFunction('f302');
printFunc($rf);
$rc = new ReflectionClass('C');
printClass($rc);
$rc = new ReflectionClass('CT');
printClass($rc);
$rc = new ReflectionClass('TC');
printClass($rc);
$rf = new ReflectionFunction('ff');
$rps = $rf->getParameters();
foreach ($rps as $rp) {
  var_dump($rp->getTypeText());
  if ($rp->getClass() !== null) {
    var_dump($rp->getClass()->getName());
  } else {
    var_dump("");
  }
}
}
