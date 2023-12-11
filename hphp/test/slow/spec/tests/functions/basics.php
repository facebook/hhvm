<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

// Function names are not case-sensitive

//function f() { echo "f\n"; }
//function F() { echo "F\n"; }  // F is equivalent to f



// function having no declared parameters
<<__DynamicallyCallable>>
function f1(...$argList)
:mixed{
    echo "f1: # arguments passed is ".count($argList)."\n";

    foreach ($argList as $k => $e)
    {
        $t = HH\is_any_array($e) ? 'Array' : $e;
        $t__str = (string)($t);
        echo "\targ[$k] = >$t__str<\n";
    }
}

// function having 2 declared parameters

function f2($p1, $p2)
:mixed{
    // A NULL value doesn't prove the argument wasn't passed; find a better test

    echo "f2: \$p1 = ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2 = ".($p2 == NULL ? "NULL" : $p2)."\n";
}

<<__DynamicallyCallable>>
function square($v) :mixed{ return $v * $v; }

<<__EntryPoint>> function main(): void {
error_reporting(-1);

var_dump(f1()); // call f1, default return value is NULL
$f = 'f1';        // assign this string to a variable
$f();           // call f1 indirectly via $f
//"f1"();           // call f1 via the string "f1" -- Can't be a string literal!!!

// f1() = 123;  // a function return is not an lvalue

f1();
f1(10);
f1(TRUE, "green");
f1(23.45, NULL, vec[1,2,3]);

// if fewer arguments are passed than there are paramaters declared, a warning is issued
// and the parameters corresponding to each each omitted argument are undefined

try { f2(); } catch (Exception $e) { var_dump($e->getMessage()); } // pass 0 (< 2)
try { f2(10); } catch (Exception $e) { var_dump($e->getMessage()); } // pass 1 (< 2)
f2(10, 20);     // pass 2 (== 2)
f2(10, 20, 30); // pass 3 (> 2)

// some simple examples of function calls

echo "5 squared = ".square(5)."\n";
var_dump($funct = 'square');
var_dump($funct(-2.3));

echo strlen("abcedfg")."\n";
}
