<?hh

// Abstract class for expression abstract syntax tree
abstract class Exp<+T> {
  public abstract function evaluate(): T;
}

// Integer literals
class IntConst extends Exp<int> {
  public function __construct(public int $val) {}
  public function evaluate(): int {
    return $this->val;
  }
}

// Boolean literals
class BoolConst extends Exp<bool> {
  public function __construct(public bool $val) {}
  public function evaluate(): bool {
    return $this->val;
  }
}

// Conditional expression (like e1 ? e2 : e3)
class CondExp<T> extends Exp<T> {
  public function __construct(
    public Exp<bool> $cond,
    public Exp<T> $then,
    public Exp<T> $else,
  ) {}

  public function evaluate(): T {
    if ($this->cond->evaluate())
      return $this->then->evaluate();
    else
      return $this->else->evaluate();
  }
}

// Binary numeric operators
enum BinOp: int {
  Add = 0;
  Sub = 1;
  Mul = 2;
  Div = 3;
}

// Binary numeric operator expressions
class BinOpExp extends Exp<num> {
  public function __construct(
    public BinOp $op,
    public Exp<num> $exp1,
    public Exp<num> $exp2,
  ) {}

  public function evaluate(): num {
    $val1 = $this->exp1->evaluate();
    $val2 = $this->exp2->evaluate();
    switch ($this->op) {
      case BinOp::Add:
        return $val1 + $val2;
      case BinOp::Sub:
        return $val1 - $val2;
      case BinOp::Mul:
        return $val1 * $val2;
      case BinOp::Div:
        return $val1 / $val2;
    }
    return 0;
  }
}

// Pair (2-tuple) expressions
class PairExp<Tu, Tv> extends Exp<(Tu, Tv)> {
  public function __construct(public Exp<Tu> $exp1, public Exp<Tv> $exp2) {}

  public function evaluate(): (Tu, Tv) {
    $val1 = $this->exp1->evaluate();
    $val2 = $this->exp2->evaluate();
    return tuple($val1, $val2);
  }
}

// Fst expression
class FstExp<Tu, Tv> extends Exp<Tu> {
  public function __construct(public Exp<(Tu, Tv)> $exp) {}

  public function evaluate(): Tu {
    list($val1, $val2) = $this->exp->evaluate();
    return $val1;
  }
}

// Snd expression
class SndExp<Tu, Tv> extends Exp<Tv> {
  public function __construct(public Exp<(Tu, Tv)> $exp) {}

  public function evaluate(): Tv {
    list($val1, $val2) = $this->exp->evaluate();
    return $val2;
  }
}

// Example: the expression fst(true ? (42,false) : (30,true)) * 3
function ExampleExp(): Exp<num> {
  return new BinOpExp(
    BinOp::Mul,
    new FstExp(
      new CondExp(
        new BoolConst(true),
        new PairExp(new IntConst(42), new BoolConst(false)),
        new PairExp(new IntConst(30), new BoolConst(true)),
      ),
    ),
    new IntConst(3),
  );
}

// Test the object-oriented eval function
function TestOOP(): void {
  $ex = ExampleExp();
  echo 'OO result is '.$ex->evaluate();
  echo "\n";
}

// Test the instanceof-based eval function
function TestInstanceof(): void {
  $ex = ExampleExp();
  echo 'instanceof result is '.IEval($ex);
  echo "\n";
}

// Now let's use new instanceof operator to implement eval
function IEval<T>(Exp<T> $e): T {
  if ($e is IntConst) {
    return $e->val;
  } else if ($e is BoolConst) {
    return $e->val;
  } else if ($e is CondExp<_>) {
    if (IEval($e->cond)) {
      return IEval($e->then);
    } else {
      return IEval($e->else);
    }
  } else if ($e is BinOpExp) {
    $val1 = IEval($e->exp1);
    $val2 = IEval($e->exp2);
    switch ($e->op) {
      case BinOp::Add:
        return $val1 + $val2;
      case BinOp::Sub:
        return $val1 - $val2;
      case BinOp::Mul:
        return $val1 * $val2;
      case BinOp::Div:
        return $val1 / $val2;
    }
  } else if ($e is FstExp<_, _>) {
    list($val1, $val2) = IEval($e->exp);
    return $val1;
  } else if ($e is SndExp<_, _>) {
    list($val1, $val2) = IEval($e->exp);
    return $val2;
  } else if ($e is PairExp<_, _>) {
    $val1 = IEval($e->exp1);
    $val2 = IEval($e->exp2);
    return tuple($val1, $val2);
  }

  throw new Exception('Unknown expression');
}

function main(): void {
  TestOOP();
  TestInstanceof();
}
