<?hh

class C{
}
function f1(?string $x = null) :mixed{
$y = (string) $x;
 var_dump($y);
 return $y;
 }
function f2(?AnyArray $x = null) :mixed{
 $y = $x !== null ? darray($x) : dict[];
 var_dump($y);
 return $y;
 }
function f4(?int $x = null) :mixed{
$y = (int) $x;
 var_dump($y);
 return $y;
 }
function f5(?bool $x = null) :mixed{
$y = (bool) $x;
 var_dump($y);
 return $y;
 }
function f6(?float $x = null) :mixed{
$y = (float)$x;
 var_dump($y);
 return $y;
 }
function rf1($x) :mixed{
 if ($x) return f1<>;
 return 0;
 }
function rf2($x) :mixed{
 if ($x) return f2<>;
 return 0;
 }
function rf4($x) :mixed{
 if ($x) return f4<>;
 return 0;
 }
function rf5($x) :mixed{
 if ($x) return f5<>;
 return 0;
 }
function rf6($x) :mixed{
 if ($x) return f6<>;
 return 0;
 }

<<__EntryPoint>>
function main_2191() :mixed{
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
var_dump(call_user_func_array(rf1(true), vec[]));
var_dump(call_user_func_array(rf2(true), vec[]));
var_dump(call_user_func_array(rf4(true), vec[]));
var_dump(call_user_func_array(rf5(true), vec[]));
var_dump(call_user_func_array(rf6(true), vec[]));
var_dump(call_user_func_array(rf1(true), vec[null]));
var_dump(call_user_func_array(rf2(true), vec[null]));
var_dump(call_user_func_array(rf4(true), vec[null]));
var_dump(call_user_func_array(rf5(true), vec[null]));
var_dump(call_user_func_array(rf6(true), vec[null]));
}
