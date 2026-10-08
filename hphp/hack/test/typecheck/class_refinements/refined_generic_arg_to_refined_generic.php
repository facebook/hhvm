<?hh

abstract class Visitor {
  abstract const type TDown;
}

abstract class Node {
  abstract public function accept<
    TVisitor as Visitor with { type TDown = TDown },
    TDown,
  >(TVisitor $v, TDown $c): void;

  public function direct<
    TVisitor as Visitor with { type TDown = TDown },
    TDown,
  >(Node $child, TVisitor $v, TDown $c): void {
    $child->accept($v, $c);
  }
}
