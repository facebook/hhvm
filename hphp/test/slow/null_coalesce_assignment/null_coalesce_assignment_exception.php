<?hh

class A {
  <<__Const>> public int $i = 10;
  <<__Const>> public dict<int, int> $d = dict[1 => 10];
  <<__Const>> public dict<int, dict<int, int>> $dd = dict[1 => dict[2 => 3]];
}

<<__EntryPoint>>
function main() :mixed{
  echo "-------- Dict ---------------------------------------------------"."\n";
  $d = dict[];
  try {
    $d['a'][0] ??= 10;
  } catch (OutOfBoundsException $e) {
    var_dump($e->getMessage());
  }
  try {
    $d[1][0] ??= 10;
  } catch (OutOfBoundsException $e) {
    var_dump($e->getMessage());
  }

  echo "--------- Vec --------------------------------------------------"."\n";
  $v = vec[];
  try {
    $v[1][0] ??= 10;
  } catch (OutOfBoundsException $e) {
    var_dump($e->getMessage());
  }

  echo "-------- Keyset ------------------------------------------------"."\n";
  $k = keyset[];
  $k ??= 10;
  try {
    $k[1] ??= 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }
  try {
    $k[2][0] ??= 10;
  } catch (OutOfBoundsException $e) {
    var_dump($e->getMessage());
  }
  try {
    $k[2][0] = 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }

  echo "-------- BaseL -------------------------------------------------"."\n";
  try {
    $a['a'] ??= 10;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    $a['a'] = 10;
  } catch (OutOfBoundsException $e) {
    var_dump($e->getMessage());
  }

  echo "------- MutateConstProp ----------------------------------------"."\n";
  // Exists
  $a = new A();
  $a->i ??= 10;
  try {
    $a->i = 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }

  // Dim
  try {
    $a->d[2] ??= 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }
  try {
    $a->d[2] = 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }

  // prop, short-circuit ok
  $a->d[1] ??= 10;
  try {
    $a->d[1] = 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }

  // elem, short-circuit ok
  $a->dd[1][2] ??= 10;
  try {
    $a->dd[1][2] = 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }

  echo "------ Dim property does not exist -----------------------------"."\n";
  try {
    $a = new A();
    $a->g->a ??= 10;
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
  try {
    $a = new A();
    $a->g->a = 10;
    // throws generic exception
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  echo "----- Dim MW ---------------------------------------------------"."\n";
  // Dim when mcode is MW
  try {
    $d[][] = 10;
  } catch (InvalidOperationException $e) {
    var_dump($e->getMessage());
  }
}
