<?hh

interface IBox {}

class Box<T> implements IBox {
  public function __construct(public T $x)[] {}
}

enum class EBox : IBox {
  Box<int> A = new Box(42);
  Box<string> B = new Box('zuck');
}

function f(HH\MemberOf<EBox, Box<int>> $member) : int {
  return $member->x;
}

function testit(): void {
  echo f(EBox::A);
  echo "\n";
}
