<?hh
/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
class C { public $p1 = 2; }
<<__EntryPoint>> function main(): void {
error_reporting(-1);

echo "================= xxx =================\n";

$x = 123;

///*
// single-quote string literals

var_dump('');
echo '>'.''."<\n";
echo '>'.' a B c '."<\n";
echo '>'.'\'.\".\\.\$.\eXXX.\f.\n.\r.\t.\v.\101.\x41.\X41.\F.\Q.\S'."<\n";
echo '>\$x.$x'."<\n";
echo '>xxx    // this comment-like thingy really is part of the string literal
yyy
zzz'."<\n";

echo var_dump('\e');    // Length should be 2
//*/

///*
// double-quote string literals

var_dump("");
echo '>'.""."<\n";
echo '>'." a B c "."<\n";
echo '>'."\'.\".\\.\$.\eXXX.\f.\n.\r.\t.\v.\101.\x41.\X41.\F.\Q.\S"."<\n";
echo ">\$x.$x"."<\n";
echo ">xxx    // this comment-like thingy really is part of the string literal
yyy
zzz"."<\n";

// the \e test is to prove that HHVM has a bug in that it doesn't recognize this escape sequence

echo var_dump("\e");    // Length should be 1
echo var_dump("\033");  // Length should be 1
echo var_dump("\x1B");  // Length should be 1
echo var_dump("\X1b");  // Length should be 1
//*/

///*
// check all the scalar types for substitution

$a = 435;           var_dump("$a");
$b = -12.34E23;     var_dump("$b");
$c = FALSE;         var_dump("$c");
$d = TRUE;          var_dump("$d");
$e = NULL;          var_dump("$e");
$f = "blue sky";    var_dump("$f");
$b__str = (string)($b);
$c__str = (string)($c);
$d__str = (string)($d);
$e__str = (string)($e);
echo ">$a|$b__str|$c__str|$d__str|$e__str|$f<\n";

$s = sprintf("%d|%G|%s|%s|%s|%s", $a, $b, $c, $d, $e, $f);
echo ">$s<\n";

$fpvalues = vec[24.543567891234565, -2345e25, 6E-200, NAN, INF];
foreach ($fpvalues as $fpval)
{
    $fpval__str = (string)($fpval);
    echo ">$fpval__str<--- o/p from string substition\n";
    $s = sprintf("%.14G", $fpval);
    echo ">$s<--- using o/p from sprintf with hard-coded precision\n";
//  $s = sprintf("%.*G", 14, $fpval);
//  echo ">$s<--- using o/p from sprintf with variable precision\n";
}

$fpval = NAN;
$fpval__str = (string)($fpval);
echo ">$fpval__str<--- o/p from string substition\n";
$s = sprintf("%.14F", $fpval);
echo ">$s<--- using o/p from sprintf with hard-coded precision\n";
//*/

///*
// show that the parser must form the longest possible variable name and that
// for unknown variables a "" is substituted

$z = -34;
$zz = "ABC";
$zzz = TRUE;
$zzzz = 567e12;
try {
  echo ">$zX|$z X|$zz_|$zz _|$zzz3|$zzz 3|$zzzz+|$zzzz +<\n";
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump("$zX");
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump("$zz_");
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump("$zzz3");
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
try {
  $zzzz__str = (string)($zzzz);
  var_dump("$zzzz__str+");
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}

// $s not preceding a variable name are used verbatim
echo ">$1|$&<\n";
//*/

///*
// use arrays and array elements

$colors = vec["red", "white", "blue"];
try { echo "\colors contains >$colors<\n"; } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { echo "\colors[1] contains >$colors[1]<\n"; } catch (Exception $e) { echo $e->getMessage()."\n"; }
// whitespace permitted, but semantics change
try { echo "\colors [1] contains >$colors [1]<\n"; } catch (Exception $e) { echo $e->getMessage()."\n"; }
//echo "\colors[1] contains >$colors[ 1]<\n";   // whitespace not permitted
//echo "\colors[1] contains >$colors[1 ]<\n";   // whitespace not permitted
try { var_dump("$colors[1]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$colors[01]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$colors[0x1]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$colors[0X1]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }

$index = 2;
echo "\$colors[$index] contains >$colors[$index]<\n";
$indices = vec[2, 1, 0];
// echo "\colors[$indices[0]] contains >$colors[$indices[0]]<\n"; // the subscript cannot itself be
// other than a simple variable
//*/

///*
$a1 = vec[10,20];
$a2 = vec[FALSE,10.3,NULL];
try { echo ">$a1|$a2<\n"; } catch (Exception $e) { echo $e->getMessage()."\n"; }

// use class properties




$myC = new C();

//echo "\$myC = >$myC<\n";  // can't use an object instance
echo "\$myC->p1 = >$myC->p1<\n";
$zzz__str = (string)($zzz);
$zzzz__str = (string)($zzzz);
//echo "\$myC ->p1 = >$myC ->p1<\n";    // whitespace not permitted
//echo "\$myC-> p1 = >$myC-> p1<\n";    // whitespace not permitted

//echo "\colors[$indices[$myC->p1]] contains >$colors[$indices[$myC->p1]]<\n"; // not permitted
//*/

///*
// use brace-delimited expressions

// braces can be use around varible names to stop a longer name being formed

echo ">{$z}X|$z X|{$zz}_|$zz _|{$zzz__str}3|$zzz__str 3|{$zzzz__str}+|$zzzz__str +<\n";
//*/
// braces having no special meaning are used verbatim

echo ">{}|{q}|}|{<\n";

$a = 10;
echo "{$a  }\n";        // trailing white space is ignored
}
