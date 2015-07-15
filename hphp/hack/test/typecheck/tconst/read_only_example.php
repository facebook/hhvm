<?hh // strict

// This is an example of a pattern I see being useful with type constants

interface ReadOnlyRef<+T> {
  public function get(): T;
}

abstract class Ref implements ReadOnlyRef<this::T> {
  abstract const type T;

  public function __construct(private this::T $val) {}

  public function set(this::T $val): this {
    $this->val = $val;
    return $this;
  }

  public function get(): this::T {
    return $this->val;
  }
}

class IntRef extends Ref {
  const type T = int;
}
class StrRef extends Ref {
  const type T = string;
}

class ShapeRef extends Ref {
  const type T = shape('x' => int);
}

function reads_ref(Vector<ReadOnlyRef<arraykey>> $refs): Vector<arraykey> {
  return $refs->map($ref ==> $ref->get());
}

function test_reads_ref(): Vector<arraykey> {
  $refs = Vector {};
  $refs[] = new IntRef(0);
  $refs[] = new StrRef('');

  return reads_ref($refs);
}
