<?hh // strict

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 0;

  <<InferFlows>>
  public function setb(int $n): void {
    $this->b = $n;
  }

  <<InferFlows>>
  public function is_a_pos(): bool {
    if ($this->a > 0) {
      return true;
    } else {
      return false;
    }
  }
}

<<InferFlows>>
function dbl(int $x): int {
  $x += $x;
  return $x;
}

function flow_a_to_b(C $c): void {
  $n = dbl($c->a);
  $c->setb($n);
}

function flow_b_to_b(C $c): void {
  $c->setb($c->b);
}

function flow_bot_to_b(C $c): void {
  $c->setb(dbl(dbl(42)));
}

function indirect_flow_a_to_b(C $c): void {
  if ($c->a > 0) {
    $c->setb(42);
  }
}

function indirect_flow_a_to_b_bis(C $c1, C $c2): void {
  if ($c1->is_a_pos()) {
    $c2->setb($c2->b + 1);
  }
}
