<?hh
function foo(Vector<int> $z): Vector<int> {
   return $z;
 }
 $rf = new ReflectionFunction('foo');
 var_dump($rf->getReturnTypeText());
 class C {
   function goo(): int {
 return 0;
 }
 }
 $rc = new ReflectionClass('C');
 $rm = $rc->getMethod('goo');
 var_dump($rm->getReturnTypeText());
 class C1 extends C {
   function goo() {
 return 0;
 }
 }
 $rc = new ReflectionClass('C1');
 $rm = $rc->getMethod('goo');
 var_dump($rm->getReturnTypeText());
 class C2 extends C1 {
   function goo(): string {
 return '0';
 }
 }
 $rc = new ReflectionClass('C2');
 $rm = $rc->getMethod('goo');
 var_dump($rm->getReturnTypeText());
 interface I {
   function m(): string;
 }
 $rc = new ReflectionClass('I');
 $rm = $rc->getMethod('m');
 var_dump($rm->getReturnTypeText());
 interface I1<T> {
   function m(): T;
 }
 $rc = new ReflectionClass('I1');
 $rm = $rc->getMethod('m');
 var_dump($rm->getReturnTypeText());
 trait T {
   function t(): C {
     return new C();
   }
 }
 class UseT {
   use T;
 }
 $rc = new ReflectionClass('UseT');
 $rm = $rc->getMethod('t');
 var_dump($rm->getReturnTypeText());
