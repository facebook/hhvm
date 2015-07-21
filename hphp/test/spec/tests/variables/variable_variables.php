<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
echo "================== simple cases ====================\n";

$color = "red";
echo "\$color = $color\n";

$$color = 123;          // 2 consecutive $s
echo "\$red = $red\n";  // ==> $red = 123
var_dump($$color);

echo "================== multiple expansion ====================\n";

$x = 'ab';
$ab = 'fg';
$fg = 'xy';

$$$$x = 'Hello';        // looks like a unary operator, and associates R->L
//$$$($x) = 'Hello';    // However, CAN'T use grouping parens to document that!!!
echo "\$xy = $xy\n";    // ==> $xy = Hello
$                       // can have arbitrary white space separators
 $
 $ $x = 'Hello';
echo "\$xy = $xy\n";

${${${$x}}} = 'Hello';
echo "\$xy = $xy\n";

var_dump($x);
var_dump($ $x);
var_dump($ $ $x);
var_dump($ $ $ $x);
//*/

///*
echo "================== Using non-variable operands to $ ====================\n";

const CON = 'v';
//$CON = 5;             // seen as 1 token ($CON), not as $ and CON
//$ CON = 5;                // syntax error, unexpected 'CON' (T_STRING),
                        // expecting variable (T_VARIABLE) or '$'

// Without the {}, the operand of $ must begin with a variable name (which
// excludes constants) // or another $
//*/

///*
echo "================== Use various scalar types as $'s operand ====================\n";

// $'s operand can be a value of any scalar type, but NOT a literal

// string operand

$v1 = 'abc';
$$v1 = '$v1 = \'abc\'';
echo "\$abc = $abc\n";
var_dump($$v1);

// int operand

$v2 = 3;
$$v2 = '$v2 = 3';
var_dump($$v2);
${$v2} = '$v2 = 3';
var_dump(${$v2});
//$3 = '$v2 = 3';
$ {  3  } = '$v2 = 3';
var_dump(${3});

// float operand

$v3 = 9.543;
$$v3 = '$v3 = 9.543';
var_dump($$v3);

// bool operand

$v4 = TRUE;
$$v4 = '$v4 = TRUE';
var_dump($$v4);
$v5 = FALSE;
$$v5 = '$v5 = FALSE';
var_dump($$v5);

// null operand

$v6 = NULL;
$$v6 = '$v6 = NULL';
var_dump($$v6);

//var_dump($GLOBALS);

function f()
{
    // the following work, but the name $'abc' is created in the local scope;
    // it certainly isn't in the Globals array. However, given the global declaration,
    // the name $'3' does designated the global by that name.

    $v11 = 'abc';
    $$v11 = '$v11 = \'abc\'';
    echo "\$abc = $abc\n";
    var_dump($$v11);

    global ${3};

    $v12 = 3;
    $$v12 = '$v12 = 3';     // changes the global
    var_dump($$v12);
}

f();

//var_dump($GLOBALS);
//*/

///*
echo "================== complex cases, [] ====================\n";

$v = array(10, 20);
$a = 'v';
$$a[0] = 5;             // [] has higher precedence, so op of $ is $ary[0]
                        // but no parens are allowed to document this
var_dump($v);
unset($v, $a);

$v = array(10, 20);
$a = 'v';
${$a[0]} = 5;           // equivalent to above, just has explicit operand
var_dump($v);
unset($v, $a);

$v = array(10, 20);
$a = 'v';
${$a}[0] = 5;           // override []'s getting first shot, ==> $v[0] = 5
var_dump($v);
unset($v, $a);
//*/

class C1
{
    public static $pr1 = 'v';
    public $pr2;

    public function __toString()
    {
        return 'w';
    }
}

///*
echo "================== complex cases, :: ====================\n";

var_dump(C1::$pr1);
${C1::$pr1} = 5;    // okay with {}
//$C1::$pr1 = 5;    // error: Undefined variable: C1, as longest token that can be formed
                // is $C1
//$ C1::$pr1 = 5;   // insert space so sees 2 tokens: $ and C1. error: syntax error,
                // unexpected 'C1' (T_STRING), expecting variable (T_VARIABLE) or '$'
                // Doesn't seem to accept a qualified name here (which would exclude a
                // namespace prefix as well)
var_dump($v);
unset($v);
//*/

///*
echo "================== complex cases, -> ====================\n";

$c1 = new C1;
$c1->pr2 = 'v';

var_dump($c1->pr2);
var_dump($c1);

$$c1->pr2 = 6;          // $w => stdClass { ["pr2"]=>int(6) }
//var_dump($GLOBALS);
${$c1}->pr2 = 7;        // $w => stdClass { ["pr2"]=>int(7) }
//var_dump($GLOBALS);

// The 2 cases above are equivalent. Here's what's happening:
// $c1 is converted to a string via __toString, which gives 'w'.
// The designated variable becomes $w, which does not exist, so it looks like
// it has a value of NULL. Then, when the -> is applied, we get a instance of stdClass.
// The problem then is that the $ operator takes precedence over the ->, which wasn't
// what I expected.

${$c1->pr2} = 8;        // $v = 8
//var_dump($GLOBALS);

unset($v, $w);
//*/

///*
echo "----------------------\n";

function ff() { return "xxx"; }

$res = ff();
$$res = 777;
echo "\$xxx = $xxx\n";
//*/
