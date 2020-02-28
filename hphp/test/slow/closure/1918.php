<?hh


<<__EntryPoint>>
function main_1918() {
$a = function ($v) {
 return $v > 2;
 }
;
 echo $a(4)."
";
 echo call_user_func_array($a, varray[4]);
}
