<?hh

class Other {}
class A {}
function foo(): void {
  /*range-start*/
  $a = new A();
  $x =shape('a' => 2, 'b' => $a)/*range-end*/;
}
