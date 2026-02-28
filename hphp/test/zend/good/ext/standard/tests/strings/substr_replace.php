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
$repl = vec["bala "];
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true)."")."\n";
(string)(var_dump(substr_replace($str, $repl, $start)))."\n";
echo "\n";
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true)."")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

echo "\n";
echo "\n";
echo "\n";



$str = vec["ala portokala"];
$repl = vec["bala "];
$start = vec[4];
$len = vec[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true)."")."\n";
(string)(var_dump(substr_replace($str, $repl, $start)))."\n";
echo "\n";

$len = vec[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

$len = vec[0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

$len = vec[-2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).", ".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";
echo "\n";




$str = vec["ala portokala"];
$repl = "bala ";
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start)))."\n";
echo "\n";
echo "\n";



$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = 4;
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = 4;
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = 4;
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = 4;
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = 4;
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";




$str = vec["ala portokala"];
$repl = "bala ";
$start = vec[4];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start)))."\n";
echo "\n";
echo "\n";



$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";


echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";


$str = vec["ala portokala"];
$repl = "bala ";
$start = vec[4,2];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start)))."\n";
echo "\n";
echo "\n";



$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = 3;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = 0;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = -2;
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";



echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";


$str = vec["ala portokala"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start)))."\n";
echo "\n";
echo "\n";



$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = vec[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = vec[0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = vec[-2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[-2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";


echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";
echo "\n";


$str = vec["ala portokala"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[3,2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start)))."\n";
echo "\n";
echo "\n";



$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = vec[3,2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[3,2];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = vec[0,0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[0,0];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";

$str = vec["ala portokala", "try this"];
$repl = vec["bala "];
$start = vec[4,2];
$len = vec[-2,-3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
echo "\n";


$str = vec["ala portokala", "try this"];
$repl = "bala ";
$start = vec[4,2];
$len = vec[-2,-3];
echo str_replace("\n","","substr_replace(".var_export($str,true).", ".var_export($repl,true).",".var_export($start,true).", ".var_export($len,true).")")."\n";
(string)(var_dump(substr_replace($str, $repl, $start, $len)))."\n";
echo "\n";
}
