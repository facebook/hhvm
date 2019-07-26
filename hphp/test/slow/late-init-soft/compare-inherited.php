<?hh

class A {
  <<__SoftLateInit>> public int $x;
}

class B extends A {}

<<__EntryPoint>> function test() {
  $a = new B();
  $a->x = 1;

  $b = new B();
  try {
    var_dump($a === $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $b = new B();
  try {
    var_dump($a !== $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $b = new B();
  try {
    var_dump($a < $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $b = new B();
  try {
    var_dump($a <= $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $b = new B();
  try {
    var_dump($a == $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $b = new B();
  try {
    var_dump($a != $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $b = new B();
  try {
    var_dump($a >= $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $b = new B();
  try {
    var_dump($a > $b);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
