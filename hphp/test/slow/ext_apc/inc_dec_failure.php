<?hh

<<__EntryPoint>>
function main_inc_dec_failure() :mixed{
$res = null;
apc_delete('test');
var_dump(apc_inc('test', 1, inout $res));
var_dump(apc_dec('test', 1, inout $res));

apc_store('x', 'x');

var_dump(apc_inc('x', 1, inout $res));
var_dump(apc_dec('x', 1, inout $res));

apc_store('numeric_str', '1');

var_dump(apc_inc('numeric_str', 1, inout $res));
var_dump(apc_dec('numeric_str', 1, inout $res));
}
