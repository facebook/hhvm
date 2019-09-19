<?hh

class C{
}
function f1(string $x = null) {
$y = (string) $x;
 var_dump($y);
 return $y;
 }
function f2(array $x = null) {
$y = (array) $x;
 var_dump($y);
 return $y;
 }
function f4(int $x = null) {
$y = (int) $x;
 var_dump($y);
 return $y;
 }
function f5(bool $x = null) {
$y = (bool) $x;
 var_dump($y);
 return $y;
 }
function f6(double $x = null) {
$y = (double) $x;
 var_dump($y);
 return $y;
 }
function rf1($x) {
 if ($x) return 'f1';
 return 0;
 }
function rf2($x) {
 if ($x) return 'f2';
 return 0;
 }
function rf4($x) {
 if ($x) return 'f4';
 return 0;
 }
function rf5($x) {
 if ($x) return 'f5';
 return 0;
 }
function rf6($x) {
 if ($x) return 'f6';
 return 0;
 }

<<__EntryPoint>>
function main_2191() {
var_dump(f1());
 var_dump(f2());

var_dump(f4());
 var_dump(f5());
 var_dump(f6());
var_dump(f1(null));
 var_dump(f2(null));
var_dump(f4(null));
 var_dump(f5(null));
 var_dump(f6(null));
var_dump(call_user_func(rf1(true)));
var_dump(call_user_func(rf2(true)));
var_dump(call_user_func(rf4(true)));
var_dump(call_user_func(rf5(true)));
var_dump(call_user_func(rf6(true)));
var_dump(call_user_func(rf1(true), null));
var_dump(call_user_func(rf2(true), null));
var_dump(call_user_func(rf4(true), null));
var_dump(call_user_func(rf5(true), null));
var_dump(call_user_func(rf6(true), null));
var_dump(call_user_func_array(rf1(true), array()));
var_dump(call_user_func_array(rf2(true), array()));
var_dump(call_user_func_array(rf4(true), array()));
var_dump(call_user_func_array(rf5(true), array()));
var_dump(call_user_func_array(rf6(true), array()));
var_dump(call_user_func_array(rf1(true), array(null)));
var_dump(call_user_func_array(rf2(true), array(null)));
var_dump(call_user_func_array(rf4(true), array(null)));
var_dump(call_user_func_array(rf5(true), array(null)));
var_dump(call_user_func_array(rf6(true), array(null)));
}
