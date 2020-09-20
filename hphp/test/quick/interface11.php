<?hh

class A {}
interface I {
  public function a($x, arraylike $a = varray[]);
  public function b(arraylike $a = null, $x);
  public function c($x, A $a1, A $a2=null, A $a3, $y);
  public function d(arraylike $a = null, $x=0, $y);
}
class B implements I {
  public function a($x, arraylike $a = varray[]) {}
  public function b(arraylike $a = null, $x) {}
  public function c($x, A $a1, A $a2=null, A $a3, $y) {}
  public function d(arraylike $a = null, $x, $y=0) {}
}
class C implements I {
  public function a($x=0, arraylike $a = null) {}
  public function b(arraylike $a = varray[], $x=0) {}
  public function c($x, A $a1=null, A $a2, A $a3=null, $y, $z=0) {}
  public function d(arraylike $a = null, $x, $y) {}
}
<<__EntryPoint>> function main(): void {
print "Test begin\n";
print "Test end\n";
}
