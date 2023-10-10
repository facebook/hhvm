<?hh

/*
 * SETUP FUNCTIONS AND CLASSES
 */

/* VS prints out comparisons and source locations in case of failure */

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "FAILED: $y\n"; var_dump(debug_backtrace()); }
}

function n_dump($x) :mixed{
  $d = debug_backtrace();
  $n = $d[0]['line'];
  echo("n_dump from line $n:\n");
  var_dump($x);
}

/* Array class with weird get and set routines to experiment with */

class CounterArray
{
  private $container = varray[];
  public $counter = 0;

  public function incrementCounter () :mixed{
    $this->counter++;
    return $this->counter;
  }
}

/* More setup */

function f0(): ?int { return null; }
function fortyseven(): int { return 47; }

class Foo2 {
    const bar3 = 'I three am a constant';
    public $vbar = 'Who am I';
}

class Foo {
    const bar = 'I am a constant';
    const bar2 = 'I too am a constant';
    public function __construct() {
      $this->foo2 = new Foo2;
    }
}

/*
 * TEST FUNCTIONS
 */

function test_basic(): void {
  echo("test_basic()\n");

  $x0 ??= 42;
  VS($x0 ??= 43, 42);
  VS($x0, 42);

  $x1 = null;
  VS($x1 ??= 43, 43);
  VS($x1, 43);

  $x2 = f0();
  VS($x2 ??= 44, 44);
  VS($x2, 44);

  VS($x_global ??= 72, 72);
  VS($x_global, 72);

  $x_string = "forty_seven";
  VS($x_string ??= 73, "forty_seven");
  VS($x_string, "forty_seven");

  VS($forty_seven ??= 48, 48);
  VS($forty_seven, 48);
}

function test_falsey(): void {
  echo("test_falsey()\n");

  $x0 = 0;
  VS($x0 ??= 71, 0);
  VS($x0, 0);

  $x1 = false;
  VS($x1 ??= 72, false);
  VS($x1, false);

  $x2 = '';
  VS($x2 ??= 73, '');
  VS($x2, '');
}

function test_array_get(): void {
  echo("test_array_get()\n");

  $arr = darray[];
  $i = -1;
  VS($arr[++$i] ??= 42, 42); // Evaluates $i once
  VS($i, 0);

  $arr[1] = 71;
  $arr[++$i] = $arr[++$i] ?? 43; // Evaluates $i twice
  VS($i, 2);

  $arr[2] ??= 44;
  $arr[3] ??= 45;
  VS($arr[++$i] ??= 72, 45); // Evaluates $i once
  VS($i, 3);

  $arr[4] = 46;
  $i = 4;
  VS($arr[++$i] ??= fortyseven(), 47); // Evaluates $i once
  VS($i, 5);

  $i = 6;
  VS($arr[$i] ??= ++$i, 7); // Sets $arr at 6 to 7 (++ does not affect LHS)
  VS($i, 7);

  $carr = new CounterArray;
  $carr->counter = 99;
  VS($arr[$carr->incrementCounter()] ??= 48, 48);
  VS($carr->counter, 100);

  // Check that the right lvalues were set to the right things
  echo('test_array_get() $arr:');
  echo("\n");
  n_dump($arr);
}

function test_object_get(): void {
  echo("test_object_get()\n");

  /* Play with CounterArray */

  $obj = new CounterArray;
  VS($obj->addme ??= "added", "added");
  VS($obj->counter ??= "don't add me", 0);

  $obj->addmetoo ??= () ==> print("add me too");
  $fortytwo = 42;
  VS($obj->$fortytwo ??= "addmethree", "addmethree");

  // Check that the right lvalues were set to the right things
  echo('test_object_get() CounterArray $obj:');
  echo("\n");
  n_dump($obj);
  echo("\n");
}

function test_associativity(): void {
  echo("test_associativity()\n");

  $x2 = 42;
  VS($x0 ??= $x1 ??= $x2 ??= 71, 42);
  VS($x0, 42);
}

function test_multi_dim_basic(): void {
  echo("test_multi_dim_basic()\n");

  $arr = darray[4 =>
    darray[2 =>
      darray[]
    ]];
  $x = 4;
  $y = 0;
  $z = 2;
  $q = 0;

  VS($arr[$x + $y][$z + $q][Foo::bar] ??= 42, 42);
  n_dump($arr); // Sets $arr[4][2] to 42 at 'I am a constant'

  $obj = new Foo;
  VS($obj->foo2->vbar ??= 71, 'Who am I'); // Doesn't set to 71
  n_dump($obj);

  $obj = new Foo;
  VS($obj->foo2->vbar2 ??= 43, 43); // Does set vbar2 to 42
  n_dump($obj);

  $obj = new Foo;
  // Strangely enough, this sets $bar2 (bar2 without the `$` is a constant)
  VS($obj->foo2->bar2 ??= 42, 42);
  n_dump($obj);
}

function test_null_base(): void {
  echo("test_null_base()\n");
  $arr = darray[]; $arreq = darray[];
  VS($arr[1] ??= 'hello', 'hello');
  VS($arreq[1] = 'hello', 'hello'); // Consistent with plain ol' equals
  VS($arr, $arreq); // $arr and $arreq get set consistently by `??=` and `=`
  $obj = new stdClass(); $objeq = new stdClass();
  VS($obj->foo ??= 'hello', 'hello'); // Also warns but doesn't fatal
  VS($objeq->foo = 'hello', 'hello'); // Also consistent
  n_dump($obj == $objeq);
}

/* Mutating variables that appear in indicies and props on the left hand side
   should not change what indices or props get set in the null case */
function test_multi_dim_lvars(): void {
  echo("test_multi_dim_lvars()\n");

  $arr = darray[4 => darray[2 => darray[]]];
  $x = 4;
  $y = 0;
  $z = 2;
  $q = 0;
  $w = 1;

  VS($arr[$x][$z][$q] ??= ($x = 2), 2);
  n_dump($arr);
  $arr[$x] = darray[$z => darray[]]; VS($arr[$x][$z][$w] ??= ($w = 3), 3);
  n_dump($arr);
  $arr[$x][$z][$w] ??= 5;
  VS($arr[$x][$z][$w] ??= ($w = 'Not me'), 5);
  n_dump($arr);
  VS($arr[$x][$z][Foo::bar] ??= 'Yay!', 'Yay!');
  n_dump($arr);
  VS($arr[$x][$z][Foo::bar2] ??= ($x = 3), 3);
  n_dump($arr);

  $a = "foo2";
  $b = "vbar2";
  $obj = new Foo;
  VS($obj->$a->$b ??= ($a = "Don't at me"), "Don't at me");
  n_dump($obj);

  $a = "foo2";
  $b = "vbar2";
  $obj = new Foo;
  VS($obj->$a->$b ??= ($b = "Don't at me"), "Don't at me");
  n_dump($obj);
}

function speak($s): string {
  echo "$s\n";
  return $s;
}

class Shoo {
    public $b = darray[];
}

function test_multi_dim_side_effects(): void {
  echo("test_multi_dim_side_effects()\n");

  $obj = new Shoo;
  $x = darray['a' => $obj];

  VS($x[speak('a')]->b[speak('c')] ??= 42, 42);
  n_dump($x);
}
<<__EntryPoint>>
function entrypoint_null_coalesce_assignment(): void {
  /*
   * CALL TEST FUNCTIONS
   */

  test_basic();
  echo "\n";
  test_falsey();
  echo "\n";
  test_array_get();
  echo "\n";
  test_object_get();
  echo "\n";
  test_associativity();
  echo "\n";
  test_multi_dim_basic();
  echo "\n";
  test_null_base();
  echo "\n";
  test_multi_dim_lvars();
  echo "\n";
  test_multi_dim_side_effects();
  echo "\n";
}
