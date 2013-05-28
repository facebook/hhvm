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
  $meths = array();
  foreach($rms as $rm) {
    $meths[$rm->getName()] = $rm;
  }
  ksort($meths);
  foreach($meths as $meth) {
    printFunc($meth);
  }
  $rps = $rc->getProperties();
  $props = array();
  foreach($rps as $rp) {
    $props[$rp->getName()] = $rp;
  }
  ksort($props);
  foreach($props as $prop) {
    var_dump($prop->getTypeText());
  }
}
function f() {
}
$rf = new ReflectionFunction('f');
printFunc($rf);
function f1(int $t) {
}
$rf = new ReflectionFunction('f1');
printFunc($rf);
function f2(@string $s) {
}
$rf = new ReflectionFunction('f2');
printFunc($rf);
function f3(?:xhp:hello $x) {
}
$rf = new ReflectionFunction('f3');
printFunc($rf);
function f100(): int {
}
$rf = new ReflectionFunction('f100');
printFunc($rf);
function f101(): string {
}
$rf = new ReflectionFunction('f101');
printFunc($rf);
function f102(): Vector<:xhp:element> {
}
$rf = new ReflectionFunction('f102');
printFunc($rf);
function f200((string, Template<A, B, ?C>, ?int, Vector<Map<C, B>>) $tuple): ?int {
}
$rf = new ReflectionFunction('f200');
printFunc($rf);
function f201((function (@int, Map<string, Map<int, Vector<string>>>):Vector<C>) $i): ClassA {
}
$rf = new ReflectionFunction('f201');
printFunc($rf);
function f202(:xhp:html-element $html): Map<string, Vector<:xhp:html-element>> {
}
$rf = new ReflectionFunction('f202');
printFunc($rf);
function f203((int, Vector<string>) $tupple): array<ClassA> {
}
$rf = new ReflectionFunction('f203');
printFunc($rf);
function f204((function (int): Vector<string>) $f): array<string, ClassA> {
}
$rf = new ReflectionFunction('f204');
printFunc($rf);
function f300<X, Y>(Y $y, ?double $d): X {
}
$rf = new ReflectionFunction('f300');
printFunc($rf);
function f301<X, Y>((function (): Vector<Y>) $f): array<string, X> {
}
$rf = new ReflectionFunction('f301');
printFunc($rf);
function f302<X, Y>((Y, X, double, string) $f): ?Y {
}
$rf = new ReflectionFunction('f302');
printFunc($rf);
class C {
  public @int $a;
  public ?string $b;
  public Vector<C> $v;
  public (string, (function(?int, (string, string)):void)) $c;
  public function __construct(?int $i, (int, string) $c) {
}
  static public function m1() {
}
  public function m2(@double $d) : void {
}
  static public function m3(Vector<Map<int, Vector<?string>>> $v, :xhp:html $x) : @array<int, ?Vector<string>> {
}
  public function m4((function(@int, (string, string)): void) $v) : array<Map<string, :xhp:html>> {
}
}
$rc = new ReflectionClass('C');
printClass($rc);
class CT<X, Y> {
  public @int $a;
  public ?string $b;
  public Vector<X> $v;
  public (string, (function(?int, (X, Y)):void)) $c;
  public function __construct(?int $i, (int, X) $c) {
}
  static public function m1() {
}
  public function m2(@Y $d) : void {
}
  static public function m3(Vector<Map<int, Vector<?X>>> $v, :xhp:html $x) : @array<Y, ?Vector<string>> {
}
  public function m4((function(@int, (X, string)): void) $v) : array<Map<Y, :xhp:html>> {
}
}
$rc = new ReflectionClass('CT');
printClass($rc);
trait T {
  static public function m1() {
}
  public function m2(@int $d) : void {
}
  static public function m3(Vector<Map<int, Vector<?C>>> $v, :xhp:html $x) : @array<C, ?Vector<string>> {
}
  public function m4((function(@int, (double, string)): void) $v) : array<Map<CT<int, string>, :xhp:html>> {
}
}
class TC {
  use T;
}
$rc = new ReflectionClass('TC');
printClass($rc);
function ff(Vector<int> $i, ?string $s, @C $c, array $a) {
}
$rf = new ReflectionFunction('ff');
$rps = $rf->getParameters();
foreach ($rps as $rp) {
  var_dump($rp->getTypeText());
  if ($rp->getClass() != null) {
      var_dump($rp->getClass()->getName());
    }
 else {
      var_dump("");
    }
}
