<?hh

function foo() :mixed{
  echo "Bad call\n";
}

function baa() :mixed{
  echo "Good call\n";
}

function live() :mixed{
  echo "Good call\n";
}

function dead() :mixed{
  echo "Bad call\n";
}

function foo1() :mixed{
  return ++LangEngineAssignexecutionorder003::$a;
}

function mod($b) :mixed{
  LangEngineAssignexecutionorder003::$x = $b;
  return 0;
}

abstract final class LangEngineAssignexecutionorder003 {
  public static $a;
  public static $x;
}

<<__EntryPoint>> function main(): void {
$b = "bb";
LangEngineAssignexecutionorder003::$a = "aa";

$bb = baa<>;

$aa = foo<>;

LangEngineAssignexecutionorder003::$a = $b;
$c = $bb;

$c();

$a1 = vec[dead<>,dead<>,dead<>];
$a2 = vec[dead<>,dead<>,live<>];
$a3 = vec[dead<>,dead<>,dead<>];

LangEngineAssignexecutionorder003::$a = vec[$a1,$a2,$a3];

$i = 0;

(LangEngineAssignexecutionorder003::$a[$i=1][++$i])();

LangEngineAssignexecutionorder003::$a = -1;

$arr = vec[vec[0,0],0];

$brr = vec[0,0,vec[0,0,0,5],0];
$crr = vec[0,0,0,0,vec[0,0,0,0,0,10],0,0];

$arr[foo1()][foo1()] = $brr[foo1()][foo1()] +
                     $crr[foo1()][foo1()];

$val = $arr[0][1];
echo "Expect 15 and get...$val\n";

LangEngineAssignexecutionorder003::$x = vec[vec[0],0];

$x1 = vec[vec[1],1];
$x2 = vec[vec[2],2];
$x3 = vec[vec[3],3];
$bx = vec[10];

LangEngineAssignexecutionorder003::$x[mod($x1)][mod($x2)] = $bx[mod($x3)];

// expecting 10,3

var_dump(LangEngineAssignexecutionorder003::$x);
}
