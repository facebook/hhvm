<?hh // strict

function test(): void {
  var_dump(foo(9, 2));
  var_dump(foo(2, 7));
  var_dump(foo_locals(2, 7));
  var_dump(make_array());

  var_dump(if_conditional1(0));
  var_dump(if_conditional1(1));
  var_dump(if_conditional2(0));
  var_dump(if_conditional2(1));
  var_dump(if_conditional3(0));
  var_dump(if_conditional3(1));

  var_dump(pluseq(5));
  var_dump(uminus(5));

  var_dump(while_loop(5, 3));
  var_dump(while_loop(5, 7));
  var_dump(for_loop(5, 3));
  var_dump(for_loop(5, 7));

  var_dump(if_conditional4(0, 0));
  var_dump(if_conditional4(0, 1));
  var_dump(if_conditional4(1, 0));
  var_dump(if_conditional4(1, 1));

  var_dump('truth tables');

  var_dump(oror_no_cond(0, 0));
  var_dump(oror_no_cond(0, 1));
  var_dump(oror_no_cond(1, 0));
  var_dump(oror_no_cond(1, 1));
  var_dump(andand_no_cond(0, 0));
  var_dump(andand_no_cond(0, 1));
  var_dump(andand_no_cond(1, 0));
  var_dump(andand_no_cond(1, 1));

  var_dump(ternary_empty(0, 5));
  var_dump(ternary_empty(1, 5));
  var_dump(ternary(0, 5, 10));
  var_dump(ternary(1, 5, 10));

  var_dump(nus());

  var_dump(interesting_array(343));
  var_dump(interesting_kv_array(343));
}


function bar(int $x, int $y): int {
  return $x + $y + $x;
}

function foo(int $x, int $y): int {
  return $x + $y;
}
function foo_locals(int $x, int $y): int {
  $z = $x + $y;
  return $z + $y;
}


function indexing(array<array<int>> $x): int {
  return $x[0][2+3];
}

function make_array(): array<int> {
  $x = array();
  $x[] = 5;
  $x[] = 4;
  $x[] = 12;
  $x[0] += 1;
  ++$x[0];
  return $x;
}

function interesting_array(int $x): array<int> {
  return array(0, $x, 1);
}
function interesting_kv_array(int $x): array<string, int> {
  return array('a' => 0, 'b' => $x, 'c' => 1);
}

function if_conditional1(int $x): string {
  if ($x) {
    $y = 'true';
  } else {
    $y = 'false';
  }
  return $y;
}

function if_conditional2(int $x): int {
  if ($x) {
    return 0;
  } else {
    return 1;
  }
}

function if_conditional3(int $x): string {
  $y = 'false';
  if ($x) {
    $y = 'true';
  }
  return $y;
}

function pluseq(int $x): int {
  $x += 5;
  $x++;
  ++$x;
  return $x;
}

function uminus(int $x): int {
  return -$x;
}

function while_loop(int $x, int $y): int {
  $i = 0;
  while ($i < $x) {
    if ($i == $y) break;
    $i++;
  }
  return $i;
}

function for_loop(int $x, int $y): int {
  for ($i = 0; $i < $x; $i++) {
    if ($i == $y) break;
    if ($i != 4) continue;
    var_dump('four');
  }
  return $i;
}

function if_conditional4(int $x, int $y): string {
  if ($x && !$y) {
    $z = 'true';
  } else {
    $z = 'false';
  }
  return $z;
}

function andand_no_cond(int $x, int $y): bool {
  return $x && $y;
}
function oror_no_cond(int $x, int $y): bool {
  return $x || $y;
}

function ternary(int $x, int $y, int $z): int {
  return $x ? $y : $z;
}
function ternary_empty(int $x, int $y): int {
  return $x ?: $y;
}
/*
function dstring2(int $x): string {
  return "hello: $x";
}
*/

function make_thing(): Nus {
  return new Nus();
}

class Nus {

  public int $baz;

  public function __construct() {
    $this->baz = 5;
  }

  public function foo(int $x, int $y): int {
    return $x + $y;
  }
  public function bar(int $x): int {
    if ($x) {
      return $this->foo(1, 2);
    } else {
      return $this->baz;
    }
  }
}

interface INus {
  public function foo(int $x, int $y): void;
}

function nus(): Nus {
  $y = new Nus();
  $z = 1;
  $za = array(0);
  var_dump($y->foo(0, 1));
  var_dump($y->bar($za[0]));
  var_dump($y->bar($z));
  return $y;
}
