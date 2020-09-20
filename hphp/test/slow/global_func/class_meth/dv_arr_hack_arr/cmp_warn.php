<?hh

class A {
  static public function func1() { return 1; }
  static public function func2() { return 2; }
}

function comp($x, $y) {
  var_dump($x === $y);
  var_dump($y === $x);

  var_dump($x == $y);
  var_dump($y == $x);

  try {
    var_dump($x < $y);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
  try {
    var_dump($y < $x);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }

  try {
    var_dump($x <= $y);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
  try {
    var_dump($y <= $x);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }

  try {
    var_dump($x > $y);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
  try {
    var_dump($y > $x);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }

  try {
    var_dump($x >= $y);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
  try {
    var_dump($y >= $x);
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main() {
  print "* vec vs vec:\n";
  comp(vec[A::class, 'func1'], vec[A::class, 'func1']);
  comp(vec[A::class, 'func1'], vec[A::class, 'func2']);
  print "* clsmeth vs vec:\n";
  comp(HH\class_meth(A::class, 'func1'), vec[A::class, 'func1']);
  comp(HH\class_meth(A::class, 'func1'), vec[A::class, 'func2']);

  print "* clsmeth vs clsmeth:\n";
  comp(HH\class_meth(A::class, 'func1'), HH\class_meth(A::class, 'func1'));
  comp(HH\class_meth(A::class, 'func1'), HH\class_meth(A::class, 'func2'));
}
