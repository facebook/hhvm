<?hh

class Point
{
    private $x;
    private $y;

    public static function getPointCount()
:mixed    {
        return self::$pointCount;
    }

    public function __construct($x = 0, $y = 0)
    {
        $this->x = $x;
        $this->y = $y;
    }

    public function move($x, $y)
:mixed    {
        $this->x = $x;
        $this->y = $y;
    }

    public function translate($x, $y)
:mixed    {
        $this->x += $x;
        $this->y += $y;
    }

    public function __toString()
:mixed    {
        return '(' . $this->x . ',' . $this->y . ')';
    }
}

function f1($b) // pass-by-value creates second alias to first point
:mixed{
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b\n";

    $b->move(4, 6);         // moving $b also moves $a
    echo "After '\$b->move(4, 6)', \$b is $b\n";

    $b = new Point(5, 7);   // removes second alias from first point;
                            // then create first alias to second new point

    echo "After 'new Point(5, 7)', \$b is $b\n";
} // $b goes away, remove the only alias from second point, so destructor runs

function f2()
:mixed{
    $b = new Point(5, 7);   // create first new point, and make $b an alias to it

    echo "After 'new Point(5, 7)', \$b is $b\n";

    return $b;  // return a temporary copy, which is a new alias
                // However, as $b goes away, remove its alias
}

class C
{
    public $prop1;
    public $prop2;
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  ///*
  echo "----------------- simple assignment of handle types ----------------------\n";

  $a = new Point(1, 3);   // create first new point, and make $a an alias to it

  echo "After '\$a = new Point(1, 3)', \$a is $a\n";

  $b = $a;        // $b is a snapshot copy of $a, so create second alias to first point

  echo "After '\$b = \$a', \$b is $b\n";

  $d = clone $b;  // create second point, and make $d the first alias to that

  echo "After '\$d = clone \$b', \$d is $d\n";

  $b->move(4, 6);     // moving $b also moves $a, but $d is unchanged

  echo "After '\$b->move(4, 6)', \$d is $d, \$b is $b, and \$a is $a\n";

  $a = new Point(2, 1);   // remove $a's alias from first point
                          // create third new point, and make $a an alias to it
                          // As $b still aliases the first point, $b is unchanged

  echo "After '\$a = new Point(2, 1)', \$d is $d, \$b is $b, and \$a is $a\n";

  unset($a);  // remove only alias from third point, so destructor runs
  unset($b);  // remove only alias from first point, so destructor runs
  unset($d);  // remove only alias from second point, so destructor runs
  echo "Done\n";
  //*/

  ///*
  echo "----------------- value argument passing of handle types ----------------------\n";

  $a = new Point(1, 3);   // create first new point, and make $a an alias to it

  echo "After '\$a = new Point(1, 3)', \$a is $a\n";

  f1($a);     // $a's point value is changed, but $a still aliases first point

  echo "After 'f1(\$a)', \$a is $a\n";

  unset($a);  // remove only alias from first point, so destructor runs
  echo "Done\n";
  //*/

  ///*
  echo "----------------- value returning of handle types ----------------------\n";

  $a = f2();      // make a new alias in $a and remove the temporary alias

  echo "After '\$a = f2()', \$a is $a\n";
  unset($a);  // remove only alias from point, so destructor runs
  echo "Done\n";
  //*/

  echo "----------------- unsetting properties ----------------------\n";

  $c = new C;

  echo "at start, \$c is "; var_dump($c);

  unset($c->prop1);
  echo "after unset(\$c->prop1), \$c is "; var_dump($c);

  unset($c->prop2);
  echo "after unset(\$c->prop2), \$c is "; var_dump($c);

  unset($c);
  echo "after unset(\$c), \$c is undefined\n";
  echo "Done\n";
}
