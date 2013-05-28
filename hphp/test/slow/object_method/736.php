<?php

/*
   some random tests used for debugging fast method call and various  invoke paths
   // php53 means this feature cannot be tested under php 5.2
*/

$fix249639=0;
 // when this task is fixed or o_id on static calls

global $trace;
function f2 ($a) {
  return $a+200;
}
function f4 ($a) {
  return $a+400;
}
class B {
  public $id;
  public $x;
  function __call($name, $arguments) {
    // keep f4bogus from fatalling
    echo "Calling B object method '$name' " . implode(', ', $arguments). "\n";
  }

  function f1($a) {
    return $x=$a+11;
  }
  function f2($a) {
    return $x=$a+12;
  }
  function f4($a) {
    return $x=$a+12;
  }
  function trace($s) {
    global $trace;
    $trace = "<$s(".$this->id.")>";
  }
  private function f4helper($a) {
    return $x=$a+12;
  }
}

class G extends B {
  public $pointless;
  function __call($name, $arguments) {
    // keep f4bogus from fatalling
    echo "Calling G object method '$name' " . implode(', ', $arguments). "\n";
  }
  function __construct($i) {
    $this->id=$i;
  }
  function f($a) {
    $this->trace("G::f");
    return $a;
  }
  function f1($a) {
    return $a;
  }
  // override
  function flongerthan8($a,$b,$c) {
    return $a+$b+$c+1;
  }
  function f4($a) {
    return B::f4($a);
  }

  // check SimpleFunctionCall::outputCPPParamOrderControlled
  /// !m_valid, !m_className.empty() case
  // called method must not exist anywhere even though it
  // looks like it might
  function f4missing($a) {

    // check SimpleFunctionCall::outputCPPParamOrderControlled
    // !m_valid, !m_className.empty() cases

    // m_validClass
    echo "Calling G object 'f4missing' 3 == ";

    //php53 echo parent::f4missing(3),"\n";
    // fatals in PHP

    // !m_validClass, m_class
    $b="B";
    // $b::f4(4);
    // task 217171
    echo "static parent method B::f4, 16 == ", B::f4(4),"\n";
    // should work

    // call an non-existant method on the one object via dynamic class name
    //php53 if($fix249639) echo "Calling G object method 'f4bogus' 5 == ", $b::f4bogus(5); __call,
    // call existing method on the one object via dynamic class name
    //php53 if($fix249639) echo "Calling G object method 'f4missing' 5 == ", $b::f4missing(5);

    // $b="Bbogus"; $b::f4bogus(6);
    // report error
    // !m_validClass, !m_class // DDD need this test yet
    //echo "missing 3 ", Bbogus::f4bogus(6),"\n";
    // fatals in PHP

  }

  function f5($a) {
    return H::f4($a);
  }
  // static call
}
class H {
  function f($a) {
    global $trace;
    $trace="H::f,";
    return "";
  }
  function f3($a) {
    return "";
  }
  function f4($a) {
    return $a+12;
  }
  function f7($a) {
    return "";
  }
}

class J {
  function __call($name, $arguments) {
    echo "Calling object method '$name' " . implode(', ', $arguments). "\n";
  }
  function f6($a) {
    return "";
  }
}

function error_handler ($errnor, $errstr, $errfile, $errline) {
  // Should catch these undefined methods here, but task 333319
  // is blocking their being caught.  For now, suppress the PHP error
  // so as to match the missing HPHP one.
  //echo "error handler<<<\n";
  //var_dump($errnor, $errstr, $errfile, $errline) ;
  //echo ">>>\n";
  return true;
}
// test invoke_builtin_static_method
//echo "bar == ",
//    call_user_func_array(array('Normalizer','normalize'),array("bar")), "\n";

$g = new G(5);
// test simple function case
echo "600 == ",
     call_user_func_array('f2',array(call_user_func_array('f4',array(0)))), "\n";

// test C::o_invoke, C::o_invoke_few_args, lookup in call_user_func
// static method call (in G::f4).
echo "1 1 13 34 12 == ",$g->f(1)," ", $g->f1(1),"  ",
     $g->f2(1)," ",$g->flongerthan8(10,11,12,13,14,15,16),
     " ",$g->f4(0),"\n";
// check case insensitive
echo "1 1 13 34 12 == ",$g->F(1)," ", $g->F1(1),"  ",
     $g->F2(1)," ",$g->Flongerthan8(10,11,12,13,14,15,16),
     " ",$g->F4(0),"\n";

// check SimpleFunctionCall::outputCPPParamOrderControlled
$prev_handler=set_error_handler("error_handler");
$g->f4missing(3);
// $b="G"; $b::f4(4);


// For those dynamic cases, check:
// 1) A call to an existing method
// 2) A call to a method which exists, but not in this class (exists in methodMap)
// 3) A call to a method which does not exist anywhere

// $func="f3"; echo "{G::f3} (3) = ",$g->$func(3),"\n";
// $func="missing"; echo "{G::missing} (3) = ",$g->$func(3),"\n";
// tests direct dynamic call
$f='f';
$f1='f1';
echo "1 1 == ",$g->{$f} (1)," ", $g->{$f1} (1),"\n";
echo "1 1 == ",$g->{'F'} (1)," ", $g->{$f1} (1),"\n";

$res = call_user_func_array("H::f",array(2));
 // ok

// tests methodIndexLookup and this variety of dynamic calls
// trying to exhause f_call_user_func_array cases
$res = call_user_func_array(array($g,'f'),array(20));
 // ok
echo "dynamic call \$g->'f' $trace, 20 == $res\n";

$res= call_user_func_array(array($g,'G::f'),array(21));
 // G::G::f a bit weird
echo "dynamic call \$g->'G::f' $trace, 21 == $res\n";
//echo "dynamic call \$g->'H::f' $trace, FAIL = ",
//      call_user_func_array(array($g,'H::f'),array(22)),"\n";
 // G::H::f better break

// Test on static class, dynamic method name, static call
$f = 'f1';
echo "31 == ",G::$f(31),"\n";
 // G::f exists
$f = 'f3';
if ($fix249639) echo "<method not found>(32) == ",G::$f(32),"\n";
 // H::f3 exists, but not G::f3
$f = 'missing';
if ($fix249639) echo "<method not found>(33) == ",G::$f(33),"\n";
 // missing does not exist

// Test dynamic class, dynamic method name, static call
$cls='G';
$f = 'f1';

//php53 echo "31 == ",$cls::$f(31),"\n";
 // G::f1 exists
$f = 'f3';
//php53 if ($fix249639) echo "<method not found>(32) == ",$cls::$f(32),"\n";
 // H::f3 exists, but not G::f3
$f = 'missing';
//php53 if ($fix249639) echo "<method not found>(33) == ",$cls::$f(33),"\n";
 // missing does not exist


// test methodIndexLookupReverse
echo "dynamic call \$g->'missing' $trace, Calling G object method 'missing' 2 = ", call_user_func_array(array($g,'missing'),array(2)),"\n";
echo "dynamic call 'missing(2)' $trace, FAIL =", call_user_func_array('missing',array(2)),"\n";

// more __call testing
$j = new J();
echo "Calling object method 'missing' 3 = ";
call_user_func_array(array($j,'missing'),array(3));


// test mapping for system function names
$ourFileName = "testFile.txt";
$ourFileHandle = fopen($ourFileName, 'w') or die("can't open file");
fclose($ourFileHandle);
unlink($ourFileName);

echo "done\n";
