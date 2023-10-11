<?hh

class C {
  <<__Policied("A")>>
  public int $a = 0;

  <<__Policied("B")>>
  public int $b = 0;

  <<__InferFlows>>
  public function setb(int $n): void {
    $this->b = $n;
  }

  <<__InferFlows>>
  public function is_a_pos(): bool {
    if ($this->a > 0) {
      return true;
    } else {
      return false;
    }
  }
}

<<__InferFlows>>
function dbl(int $x): int {
  $x += $x;
  return $x;
}

<<__InferFlows>>
function flow_a_to_b(C $c): void {
  $n = dbl($c->a);
  $c->setb($n);
}

<<__InferFlows>>
function flow_b_to_b(C $c): void {
  $c->setb($c->b);
}

<<__InferFlows>>
function flow_bot_to_b(C $c): void {
  $c->setb(dbl(dbl(42)));
}

<<__InferFlows>>
function indirect_flow_a_to_b(C $c): void {
  if ($c->a > 0) {
    $c->setb(42);
  }
}

<<__InferFlows>>
function indirect_flow_a_to_b_bis(C $c1, C $c2): void {
  if ($c1->is_a_pos()) {
    $c2->setb($c2->b + 1);
  }
}
