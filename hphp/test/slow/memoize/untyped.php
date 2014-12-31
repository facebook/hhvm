<?hh
class A {
  <<__Memoize>>
  public function oneArgMeth($a) {
    static $i = 10;
    return $i++;
  }
  <<__Memoize>>
  public function multiArgMeth($a, $b, $c) {
    static $i = 20;
    return $i++;
  }
  <<__Memoize>>
  public static function oneArgStatic($a) {
    static $i = 30;
    return $i++;
  }
  <<__Memoize>>
  public static function multiArgStatic($a, $b, $c) {
    static $i = 40;
    return $i++;
  }
}

<<__Memoize>>
function oneArgTopLevel($a) {static $i = 50; return $i++;}
<<__Memoize>>
function multiArgTopLevel($a, $b, $c) {static $i = 60; return $i++;}
<<__Memoize>>
function passthrough($a) {return $a;}

class O implements HH\IMemoizeParam {
  public function __construct(public string $a) {}
  public function getInstanceKey(): string { return $this->a; }
}

echo "Test each kind of function call with one and many args\n";
$a = new A();
echo $a->oneArgMeth(1).' ';
echo $a->oneArgMeth(1).' ';
echo $a->oneArgMeth("1").' ';
echo $a->oneArgMeth("1")."\n";

echo $a->multiArgMeth(1, 2, "3").' ';
echo $a->multiArgMeth(1, 2, "3").' ';
echo $a->multiArgMeth(4, 5, null).' ';
echo $a->multiArgMeth(4, 5, null)."\n";

echo A::oneArgStatic(0).' ';
echo A::oneArgStatic(0).' ';
echo A::oneArgStatic("0").' ';
echo A::oneArgStatic("0")."\n";

echo A::multiArgStatic("1", 2, 3).' ';
echo A::multiArgStatic("1", 2, 3).' ';
echo A::multiArgStatic(4, 5.2, 6).' ';
echo A::multiArgStatic(4, 5.2, 6)."\n";

echo oneArgTopLevel(false).' ';
echo oneArgTopLevel(false).' ';
echo oneArgTopLevel(null).' ';
echo oneArgTopLevel(null)."\n";

echo multiArgTopLevel(1, "2", true).' ';
echo multiArgTopLevel(1, "2", true).' ';
echo multiArgTopLevel(4.23, null, 6).' ';
echo multiArgTopLevel(4.23, null, 6)."\n";

echo "Test arrays\n";
echo multiArgTopLevel(array(1,1,2,3), array(), array(1=>2)).' ';
echo multiArgTopLevel(array(1,1,2,3), array(), array(1=>2)).' ';
echo multiArgTopLevel(array(1,1,2,3), array("a"), array(1=>2)).' ';
echo multiArgTopLevel(array(1,1,2,3), array("a"), array(1=>2)).' ';
echo multiArgTopLevel(array(2,1,2,3), array(), array(1=>2)).' ';
echo multiArgTopLevel(array(2,1,2,3), array(), array(1=>2)).' ';
echo multiArgTopLevel(array(1,1,2,3), array(), array(2=>1)).' ';
echo multiArgTopLevel(array(1,1,2,3), array(), array(2=>1)).' ';
echo multiArgTopLevel(array(), array(1,1,2,3), array(1=>2)).' ';
echo multiArgTopLevel(array(), array(1,1,2,3), array(1=>2)).' ';
echo oneArgTopLevel(array(array(1), array(2))).' ';
echo oneArgTopLevel(array(array(1), array(2)))."\n";

echo "Test objects\n";
echo multiArgTopLevel(new O("1"), new O("1"), new O("2")).' ';
echo multiArgTopLevel(new O("1"), new O("1"), new O("2")).' ';
echo multiArgTopLevel(new O("1"), new O("2"), new O("1")).' ';
echo multiArgTopLevel(new O("1"), new O("2"), new O("1")).' ';
echo oneArgTopLevel(array(new O("foo"))).' ';
echo oneArgTopLevel(array(new O("foo"))).' ';
echo oneArgTopLevel(array(array(new O("foo")), array(new O("bar")))).' ';
echo oneArgTopLevel(array(array(new O("foo")), array(new O("bar"))))."\n";

echo "Test Hack collections\n";
echo oneArgTopLevel(Vector {1,2,3}).' ';
echo oneArgTopLevel(Vector {1,2,3}).' ';
echo oneArgTopLevel(Vector {3,2,1}).' ';
echo oneArgTopLevel(Vector {3,2,1})."\n";

echo oneArgTopLevel(ImmVector {4,5,6}).' ';
echo oneArgTopLevel(ImmVector {4,5,6}).' ';
echo oneArgTopLevel(ImmVector {6,5,4}).' ';
echo oneArgTopLevel(ImmVector {6,5,4})."\n";

echo oneArgTopLevel(Set {1,2,3}).' ';
echo oneArgTopLevel(Set {1,2,3}).' ';
echo oneArgTopLevel(Set {1,2,4}).' ';
echo oneArgTopLevel(Set {1,2,4}).' ';
echo oneArgTopLevel(Set {3,2,1}).' ';
echo oneArgTopLevel(Set {3,2,1})."\n";

echo oneArgTopLevel(ImmSet {4,5,6}).' ';
echo oneArgTopLevel(ImmSet {4,5,6}).' ';
echo oneArgTopLevel(ImmSet {4,5,7}).' ';
echo oneArgTopLevel(ImmSet {4,5,7})."\n";

echo oneArgTopLevel(Map {'a' => 1, 'b' => 2}).' ';
echo oneArgTopLevel(Map {'a' => 1, 'b' => 2}).' ';
echo oneArgTopLevel(Map {'c' => 1, 'd' => 2}).' ';
echo oneArgTopLevel(Map {'c' => 1, 'd' => 2})."\n";

echo oneArgTopLevel(ImmMap {'e' => 5, 'f' => 6}).' ';
echo oneArgTopLevel(ImmMap {'e' => 5, 'f' => 6}).' ';
echo oneArgTopLevel(ImmMap {'e' => 7, 'f' => 8}).' ';
echo oneArgTopLevel(ImmMap {'e' => 7, 'f' => 8})."\n";

echo "Test that the args are correctly passed to the real function\n";
$o = new O("obj");
echo passthrough(0) === 0 ? '1 ' : '-1 ';
echo passthrough(false) === false ? '2 ' : '-2 ';
echo passthrough(null) === null ? '3 ' : '-3 ';
echo passthrough('') === '' ? '4 ' : '-4 ';
echo passthrough(array()) === array() ? '5 ' : '-5 ';
echo passthrough(1) === 1 ? '6 ' : '-6 ';
echo passthrough(true) === true ? '7 ' : '-7 ';
echo passthrough('foo') === 'foo' ? '8 ' : '-8 ';
echo passthrough(array(1,2,3)) === array(1,2,3) ? '9 ' : '-9 ';
echo passthrough(1.2) === 1.2 ? '10 ' : '-10 ';
echo passthrough($o) === $o ? '11 ' : '-11 ';
$o = Vector {1};
echo passthrough($o) === $o ? '12 ' : '-12 ';
$o = Set {2};
echo passthrough($o) === $o ? '13 ' : '-13 ';
$o = Map {1 => 2};
echo passthrough($o) === $o ? "14\n" : "-14\n";

echo "Test that objects and strings don't collide\n";
echo oneArgTopLevel(new O('test1')).' ';
echo oneArgTopLevel('test1').' ';
echo oneArgTopLevel(new O('test2')).' ';
echo oneArgTopLevel('test2')."\n";
