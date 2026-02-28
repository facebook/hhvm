<?hh

class A {
  public $x = 123;
  <<__LateInit>> public $y;
  public $z = 'abc';
}

function test($name, $a, $b) :mixed{
  echo "==== $name ====\n";
  try {
    var_dump($a === $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a !== $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a < $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a <= $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a == $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a != $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a >= $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a > $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  try {
    var_dump($a <=> $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

<<__EntryPoint>> function main(): void {
  $a = new A();
  $a->y = 'test';
  $b = new A();
  test("never set", $a, $b);

  $b->y = 'test';
  test("set", $a, $b);

  unset($a->y);
  test("unset", $a, $b);

  $a->x = 456;
  test("short-circuit", $a, $b);
}
