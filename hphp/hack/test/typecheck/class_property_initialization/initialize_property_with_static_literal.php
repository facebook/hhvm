<?hh //strict

class Bar {
  const string THE_CONST = "bar const";
}

class Foo {
  public int $integral = 10;
  public float $decimal = 2.5;
  public int $unary_op = -1;
  public int $binary_op = 1 + 1;
  public string $string = "hello";
  public string $string2 = 'hello';
  public bool $bool = TRUE;
  public string $class_constant = Bar::THE_CONST;
  public int $conditional = Bar::THE_CONST ? 1 : 2;
  public string $ternary = Bar::THE_CONST ?: "ok";
  public (int, int, int) $tuple = tuple(1, 3, 4);
  public Vector<int> $vec = Vector { 1, 2, 3 };
  public Map<int, string> $map = Map { 1 => 'hello', 2 => 'world' };
}
