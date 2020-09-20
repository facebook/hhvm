<?hh <<__EntryPoint>> function main(): void {
$str = "try this";
$repl = "bala ";
$start = 2;
echo "\n";


echo "substr_replace('$str', '$repl', $start)\n";
var_dump(substr_replace($str, $repl, $start));
echo "\n";

$len = 3;
echo "substr_replace('$str', '$repl', $start, $len)\n";
var_dump(substr_replace($str, $repl, $start, $len));
echo "\n";

$len = 0;
echo "substr_replace('$str', '$repl', $start, $len)\n";
var_dump(substr_replace($str, $repl, $start, $len));
echo "\n";

$len = -2;
echo "substr_replace('$str', '$repl', $start, $len)\n";
var_dump(substr_replace($str, $repl, $start, $len));
echo "\n";
echo "\n";
echo "\n";


$str = "try this";
$repl = varray["bala "];
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true)."")."\n";
var_dump(substr_replace($str, $repl, $start))."\n";
echo "\n";
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true)."")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

echo "\n";
echo "\n";
echo "\n";



$str = varray["ala portokala"];
$repl = varray["bala "];
$start = varray[4];
$len = varray[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true)."")."\n";
var_dump(substr_replace($str, $repl, $start))."\n";
echo "\n";

$len = varray[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

$len = varray[0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

$len = varray[-2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";
echo "\n";




$str = varray["ala portokala"];
$repl = "bala ";
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
var_dump(substr_replace($str, $repl, $start))."\n";
echo "\n";
echo "\n";



$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = 4;
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = 4;
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = 4;
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = 4;
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";




$str = varray["ala portokala"];
$repl = "bala ";
$start = varray[4];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
var_dump(substr_replace($str, $repl, $start))."\n";
echo "\n";
echo "\n";



$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";


echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";


$str = varray["ala portokala"];
$repl = "bala ";
$start = varray[4,2];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
var_dump(substr_replace($str, $repl, $start))."\n";
echo "\n";
echo "\n";



$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";



echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";


$str = varray["ala portokala"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
var_dump(substr_replace($str, $repl, $start))."\n";
echo "\n";
echo "\n";



$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = varray[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = varray[0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = varray[-2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[-2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";


echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";


$str = varray["ala portokala"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[3,2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
var_dump(substr_replace($str, $repl, $start))."\n";
echo "\n";
echo "\n";



$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = varray[3,2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[3,2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = varray[0,0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[0,0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";

$str = varray["ala portokala", "try this"];
$repl = varray["bala "];
$start = varray[4,2];
$len = varray[-2,-3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
echo "\n";


$str = varray["ala portokala", "try this"];
$repl = "bala ";
$start = varray[4,2];
$len = varray[-2,-3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
var_dump(substr_replace($str, $repl, $start, $len))."\n";
echo "\n";
}
