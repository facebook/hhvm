<?hh

class Dummy {
}
class foo {
  public static $v = dict[Dummy::c => 'foo'];
}
interface A {
  const CONSTANT = 'CONSTANT';
}
class B implements A {
 }
class C {
  public static $A_CONSTANT = A::CONSTANT;
  public static $B_CONSTANT = B::CONSTANT;
}

<<__EntryPoint>>
function main_1610() :mixed{
var_dump(A::CONSTANT);
var_dump(B::CONSTANT);
var_dump(C::$A_CONSTANT);
var_dump(C::$B_CONSTANT);
}
