<?hh


<<__EntryPoint>>
function main_1930() {
$v=5;
call_user_func(  function() use($v)   {
 echo $v;
 }
);
$f = function() use($v) {
 echo $v;
 }
;
call_user_func($f);
call_user_func_array(  function() use($v)   {
 echo $v;
 }
, varray[]);
call_user_func($f, varray[]);
}
