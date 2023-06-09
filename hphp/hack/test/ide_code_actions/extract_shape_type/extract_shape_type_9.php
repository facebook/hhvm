<?hh

class Other {}
class A {}
function foo(): void {
  $a = new A();
  $x = /*range-start*/false || shape('a' => 2, 'b' => $a)/*range-end*/;
}
