<?hh

// expect the inner array to be defined
function f() :mixed{
  LangEngineAssignexecutionorder002::$ee = vec["array created in f()"];
  return 1;
}

abstract final class LangEngineAssignexecutionorder002 {
  public static $ee;
}


// simple case with missing element
<<__EntryPoint>> function main(): void { $f = vec["hello","item2","bye"];
list($a,,$b) = $f;
echo "A=$a B=$b\n";

// i++ evaluated first, so $d[0] is 10
$d = vec[0,10];
$i = 0;
$d[$i++] = $i*10;
// expected array is 10,10
var_dump($d);

// the f++++ makes f into 2, so $e 0 and 1 should both be 30
$e = vec[0,0];
$f = 0;
$g1 = vec[10,10];
$g2 = vec[20,20];
$g3 = vec[30,30];
$g = vec[$g1,$g2,$g3];
list($e[$f++],$e[$f++]) = $g[$f];
// expect 30,30
var_dump($e);


$i1 = vec[1,2];
$i2 = vec[10,20];
$i3 = vec[100,200];
$i4 = vec[vec[1000,2000],3000];
$i = vec[$i1,$i2,$i3,$i4];
$j = vec[0,0,0];
$h = 0;
// a list of lists
list(list($j[$h++],$j[$h++]),$j[$h++]) = $i[$h];
var_dump($j);


// list of lists with just variable assignments - expect 100,200,300
$k3 = vec[100,200];
$k = vec[$k3,300];
list(list($l,$m),$n) = $k;
echo "L=$l M=$m N=$n\n";


// expect $x and $y to be null - this fails on php.net 5.2.1 (invalid opcode) - fixed in 5.2.3
list($o,$p) = 20;
echo "O=".(string)$o." and P=".(string)$p."\n";


// list of lists with blanks and nulls expect 10 20 40 50 60 70 80
$q1 = vec[10,20,30,40];
$q2 = vec[50,60];
$q3 = vec[$q1,$q2,null,70];
$q4 = vec[$q3,null,80];

list(list(list($r,$s,,$t),list($u,$v),,$w),,$x) = $q4;
echo "$r $s $t $u $v $w $x\n";


// expect y and z to be undefined
try {
  list($y,$z) = vec[];
  echo "Y=$y,Z=$z\n";
} catch (Exception $e) { echo $e->getMessage()."\n"; }

// expect h to be defined and be 10
try {
  list($aa,$bb) = vec[10];
  echo "AA=$aa\n";
} catch (Exception $e) { echo $e->getMessage()."\n"; }


// expect cc and dd to be 10 and 30
list($cc,,$dd) = vec[10,20,30,40];
echo "CC=$cc DD=$dd\n";

LangEngineAssignexecutionorder002::$ee = vec["original array"];
}
