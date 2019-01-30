<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
$v = 0;
echo var_dump((bool)$v);
echo var_dump((boolean)$v);
echo var_dump((int)$v);
echo var_dump((integer)$v);
echo var_dump((float)$v);
echo var_dump((double)$v);
echo var_dump((real)$v);
echo var_dump((string)$v);
echo var_dump((array)$v);
echo var_dump((object)$v);

echo var_dump((binary)$v);
echo var_dump((binary)"");
echo var_dump((binary)"abcdef");

echo var_dump($v);
echo var_dump((unset)$v);
echo var_dump($v);
//*/

///*
// Test all kinds of values to see which can be converted

$ary1 = array(5 => 10, 2 => 20);
$ary2 = array(1.23, TRUE, "Hello", NULL);

$scalarValueList = array(10, -100, 0, 1.56, 0.0, 1234e200, INF, -INF, NAN, TRUE, FALSE, NULL,
 "123", 'xx', "", "0", "00", "0.000", "0ABC", "0.000ABC", $ary1, $ary2);

foreach ($scalarValueList as $v)
{
    var_dump($v);

    echo "   "; var_dump((bool)$v);
    echo "   "; var_dump((bool)$v);
    echo "   "; var_dump((int)$v);
    echo "   "; var_dump((float)$v);
    echo "   "; var_dump((string)$v);
    echo "   "; var_dump((array)$v);
    echo "   >>---"; var_dump((object)$v);
}
//*/

///*
var_dump(10/3);
var_dump((int)(10/3));              // results in the int 3 rather than the float 3.333...
var_dump((array)(16.5));            // results in an array of 1 float; [0] = 16.5
var_dump((int)(float)"123.87E3");   // results in the int 123870
//*/

///*
echo "---------------\n";

class C {}      // has no __toString method

$c1 = new C;
var_dump($c1);
//var_dump((string)$c1);

class D
{
    public function __toString()
    {
        return "AAA";
    }
}

$d1 = new D;
var_dump($d1);
var_dump((string)$d1);
//*/

///*
echo "---------------\n";

class E
{
    const CON1 = 123;               // constants irrelevent for conversion purposes
    public function f() {}          // methods irrelevent for conversion purposes
    private static $fsprop = 0;     // static properties irrelevent for conversion purposes

    private $priv_prop;
    protected $prot_prop = 12.345;
    public $publ_prop;

    public function __construct($p1)
    {
        $this->publ_prop = $p1;
    }
}

$e1 = new E(array(10, 1.2, "xxx"));
echo var_dump((bool)$e1);       // bool(true)
//echo var_dump((int)$e1);      // invalid
//echo var_dump((float)$e1);        // invalid
//echo var_dump((string)$e1);   // only works if __toString() defined
$ary = (array)$e1;
echo var_dump($ary);            // array of zero or more elements, 1 per instance property
echo var_dump((object)$e1);     // redundant; OK

foreach ($ary as $key => $val)
{
    echo "\$key: >$key<, len: " . strlen($key) . ", \$val: >$val<\n";
}
//*/

///*
echo "---------------\n";

//$infile = fopen("Testfile.txt", 'r');
$infile = STDIN;
var_dump($infile);

echo var_dump((bool)$infile);
echo var_dump((int)$infile);
echo var_dump((float)$infile);
echo var_dump((string)$infile);
echo var_dump((array)$infile);
$v = (array)$infile;
var_dump($v[0]);
var_dump(gettype($v[0]));
echo var_dump((object)$infile);
//*/

echo "---------------\n";

$str = "AaBb123$%^";
$binStr = (binary)$str;
var_dump($binStr);

$binStr = b"AaBb123$%^";
var_dump($binStr);
