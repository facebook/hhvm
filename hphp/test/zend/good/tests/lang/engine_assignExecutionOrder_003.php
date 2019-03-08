<?php
$b = "bb";
LangEngineAssignexecutionorder003::$a = "aa";

function foo()
{
echo "Bad call\n";
}

function baa()
{
echo "Good call\n";
}

$bb = "baa";

$aa = "foo";

LangEngineAssignexecutionorder003::$a = $b;
$c = $bb;

$c();

$a1 = array("dead","dead","dead");
$a2 = array("dead","dead","live");
$a3 = array("dead","dead","dead");

LangEngineAssignexecutionorder003::$a = array($a1,$a2,$a3);

function live()
{
echo "Good call\n";
}

function dead()
{
echo "Bad call\n";
}

$i = 0;

(LangEngineAssignexecutionorder003::$a[$i=1][++$i])();

LangEngineAssignexecutionorder003::$a = -1;

function foo1()
{

  return ++LangEngineAssignexecutionorder003::$a;
}

$arr = array(array(0,0),0);

$brr = array(0,0,array(0,0,0,5),0);
$crr = array(0,0,0,0,array(0,0,0,0,0,10),0,0);

$arr[foo1()][foo1()] = $brr[foo1()][foo1()] +
                     $crr[foo1()][foo1()];

$val = $arr[0][1];
echo "Expect 15 and get...$val\n";

LangEngineAssignexecutionorder003::$x = array(array(0),0);
function mod($b)
{

LangEngineAssignexecutionorder003::$x = $b;
return 0;
}

$x1 = array(array(1),1);
$x2 = array(array(2),2);
$x3 = array(array(3),3);
$bx = array(10);

LangEngineAssignexecutionorder003::$x[mod($x1)][mod($x2)] = $bx[mod($x3)];

// expecting 10,3

var_dump(LangEngineAssignexecutionorder003::$x);

abstract final class LangEngineAssignexecutionorder003 {
  public static $a;
  public static $x;
}
