<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point
{
    private static $pointCount = 0;

    private $x;
    private $y;

    public static function getPointCount()
    {
        return self::$pointCount;
    }

    public function __construct($x = 0, $y = 0)
    {
        $this->x = $x;
        $this->y = $y;
        ++self::$pointCount;

        echo "\nInside " . __METHOD__ . ", $this, point count = " . self::$pointCount . "\n\n";
    }

    public function move($x, $y)
    {
        $this->x = $x;
        $this->y = $y;
    }   

    public function translate($x, $y)
    {
        $this->x += $x;
        $this->y += $y;
    }

    public function __destruct()
    {
        --self::$pointCount;

        echo "\nInside " . __METHOD__ . ", $this, point count = " . self::$pointCount . "\n\n";
    }
///*
    public function __clone()
    {
        ++self::$pointCount;

        echo "\nInside " . __METHOD__ . ", $this, point count = " . self::$pointCount . "\n\n";
    }
//*/

    public function __toString()
    {
        return '(' . $this->x . ',' . $this->y . ')';
    }   
}

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
echo "----------------- byRef assignment of handle types ----------------------\n";

$a = new Point(1, 3);   // create first new point, and make $a an alias to it

echo "After '\$a = new Point(1, 3)', \$a is $a\n";

$c =& $a;           // make $c forever alias whatever $a aliases

echo "After '\$c =& \$a', \$c is $c, and \$a is $a\n";

$a->move(4, 6);     // moving $a also moves $c

echo "After '\$a->move(4, 6)', \$c is $c, and \$a is $a\n";

$a = new Point(2, 1);   // remove $a's alias from first point
                        // create second new point, and make $a an alias to it
                        // As $c aliases whatever $a aliases, $c's old alias to the first
                        // point is also removed, allowing the destructor to run.
                        // $c's new alias is to the new point

echo "After '\$a = new Point(2, 1)', \$c is $c, and \$a is $a\n";

unset($a);  // remove one alias from second point
echo "After 'unset(\$a)', \$c is $c\n";
unset($c);  // remove second (and final) alias from second point, so destructor runs
echo "Done\n";
//*/

///*
echo "----------------- value argument passing of handle types ----------------------\n";

function f1($b) // pass-by-value creates second alias to first point
{
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b\n";

    $b->move(4, 6);         // moving $b also moves $a
    echo "After '\$b->move(4, 6)', \$b is $b\n";

    $b = new Point(5, 7);   // removes second alias from first point;
                            // then create first alias to second new point

    echo "After 'new Point(5, 7)', \$b is $b\n";
} // $b goes away, remove the only alias from second point, so destructor runs

$a = new Point(1, 3);   // create first new point, and make $a an alias to it

echo "After '\$a = new Point(1, 3)', \$a is $a\n";

f1($a);     // $a's point value is changed, but $a still aliases first point

echo "After 'f1(\$a)', \$a is $a\n";

unset($a);  // remove only alias from first point, so destructor runs
echo "Done\n";
//*/

///*
echo "----------------- byRef argument passing of handle types ----------------------\n";

function g1(&$b)    // make $b alias whatever $a aliases
{
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b\n";

    $b->move(4, 6);         // moving $b also moves $a
    echo "After '\$b->move(4, 6)', \$b is $b\n";

    $b = new Point(5, 7);   // removes second alias from first point;
                            // then create first alias to second new point
                            // changing $b also changes $a as well, so $a's alias
                            // is also removed, alowing the destructor run

    echo "After 'new Point(5, 7)', \$b is $b\n";
} // $b goes away, remove its alias from new point

$a = new Point(1, 3);   // create first new point, and make $a an alias to it

echo "After '\$a = new Point(1, 3)', \$a is $a\n";

g1($a);     // $a is changed via change to $b

echo "After 'g1(\$a)', \$a is $a\n";
unset($a);  // remove only alias from point, so destructor runs
echo "Done\n";
//*/

///*
echo "----------------- value returning of handle types ----------------------\n";

function f2()
{
    $b = new Point(5, 7);   // create first new point, and make $b an alias to it

    echo "After 'new Point(5, 7)', \$b is $b\n";

    return $b;  // return a temporary copy, which is a new alias
                // However, as $b goes away, remove its alias
}

$a = f2();      // make a new alias in $a and remove the temporary alias

echo "After '\$a = f2()', \$a is $a\n";
unset($a);  // remove only alias from point, so destructor runs
echo "Done\n";
//*/

///*
echo "----------------- byRef returning of handle types ----------------------\n";

function & g2()
{
    $b = new Point(5, 7);   // create first new point, and make $b an alias to it

    echo "After 'new Point(5, 7)', \$b is $b\n";

    return $b;  // return as though using $a =& $b
                // as $b goes away, remove its alias
}

$a = g2();

echo "After '\$a = f2()', \$a is $a\n";
unset($a);  // remove only alias from point, so destructor runs
echo "Done\n";
//*/

echo "----------------- unsetting properties ----------------------\n";

class C
{
    public $prop1;
    public $prop2;

    public function __destruct()
    {
        echo "\nInside " . __METHOD__ . "\n\n";
    }
}

$c = new C;

echo "at start, \$c is "; var_dump($c);

unset($c->prop1);
echo "after unset(\$c->prop1), \$c is "; var_dump($c);

unset($c->prop2);
echo "after unset(\$c->prop2), \$c is "; var_dump($c);

unset($c);
echo "after unset(\$c), \$c is undefined\n";
echo "Done\n";
