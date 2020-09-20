<?hh

function comp_func($a, $b) {
        if ($a === $b) return 0;
        return ($a > $b)? 1:-1;

}
<<__EntryPoint>> function main(): void {
$a = varray[1, 6, 2, -20, 15, 1200, -2500];
$b = varray[0, 7, 2, -20, 11, 1100, -2500];
$c = varray[0, 6, 2, -20, 19, 1000, -2500];
$d = varray[3, 8,-2, -20, 14,  900, -2600];
$a_f = array_flip($a);
$b_f = array_flip($b);
$c_f = array_flip($c);
$d_f = array_flip($d);
$i = 1;
/* give nicer values */
foreach ($a_f as $k => $_) { $a_f[$k] =$k*2;}
foreach ($b_f as $k => $_) { $b_f[$k] =$k*2;}
foreach ($c_f as $k => $_) { $c_f[$k] =$k*2;}
foreach ($d_f as $k => $_) { $d_f[$k] =$k*2;}

echo "------ Test $i --------\n";$i++;// 1
var_dump(array_diff_key($a_f, $b_f));// keys -> 1, 6, 15, 1200
var_dump(array_diff_ukey($a_f, $b_f, fun("comp_func")));// 1, 6, 15, 1200

echo "------ Test $i --------\n";$i++;// 2
var_dump(array_diff_key($a_f, $c_f));// keys -> 1, 15, 1200
var_dump(array_diff_ukey($a_f, $c_f, fun("comp_func")));// 1, 15, 1200

echo "------ Test $i --------\n";$i++;// 3
var_dump(array_diff_key($a_f, $d_f));// 1, 6, 2, 15, 1200, -2500
var_dump(array_diff_ukey($a_f, $d_f, fun("comp_func")));// 1, 6, 2, 15, 1200, -2500

echo "------ Test $i --------\n";$i++;// 4
var_dump(array_diff_key($a_f, $b_f, $c_f));// 1, 15, 1200
var_dump(array_diff_ukey($a_f, $b_f, $c_f, fun("comp_func")));// 1, 15, 1200

echo "------ Test $i --------\n";$i++;// 5
var_dump(array_diff_key($a_f, $b_f, $d_f));// 1, 6, 15, 1200
var_dump(array_diff_ukey($a_f, $b_f, $d_f, fun("comp_func")));// 1, 6, 15, 1200


echo "------ Test $i --------\n";$i++;// 6
var_dump(array_diff_key($a_f, $b_f, $c_f, $d_f));// 1, 15, 1200
var_dump(array_diff_ukey($a_f, $b_f, $c_f, $d_f, fun("comp_func")));//1, 15, 1200


echo "------ Test $i --------\n";$i++;// 7
var_dump(array_diff_key($b_f, $c_f));// 7, 11, 1100
var_dump(array_diff_ukey($b_f, $c_f, fun("comp_func")));//7, 11, 1100

echo "------ Test $i --------\n";$i++;// 8
var_dump(array_diff_key($b_f, $d_f));//0, 7, 2, 11, 1100, -2500
var_dump(array_diff_ukey($b_f, $d_f, fun("comp_func")));//0, 7, 2, 11, 1100, -2500


echo "------ Test $i --------\n";$i++;// 9
var_dump(array_diff_key($b_f, $c_f, $d_f));// 7, 11, 1100
var_dump(array_diff_ukey($b_f, $c_f,  $d_f, fun("comp_func")));// 7, 11, 1000
}
