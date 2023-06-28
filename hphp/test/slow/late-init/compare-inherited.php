<?hh

class A {
  <<__LateInit>> public int $x;
}

class B extends A {}

<<__EntryPoint>> function test() :mixed{
  $a = new B();
  $a->x = 1;
  $b = new B();

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
}
