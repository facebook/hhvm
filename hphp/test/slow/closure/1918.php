<?hh


<<__EntryPoint>>
function main_1918() :mixed{
$a = function ($v) {
 return $v > 2;
 }
;
 echo (string)$a(4)."
";
 echo call_user_func_array($a, vec[4]);
}
