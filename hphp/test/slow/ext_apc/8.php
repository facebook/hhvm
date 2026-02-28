<?hh


<<__EntryPoint>>
function main_8() :mixed{
$res = null;
apc_store("ts", 12);
if (apc_dec("ts", 1, inout $res) !== 11) echo "no\n";
if (apc_dec("ts", 5, inout $res) !== 6) echo "no\n";
if (apc_dec("ts", -3, inout $res) !== 9) echo "no\n";
echo "ok\n";
}
