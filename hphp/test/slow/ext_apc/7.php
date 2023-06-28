<?hh


<<__EntryPoint>>
function main_7() :mixed{
apc_store("ts", 12);
$res = null;
if (apc_inc("ts", 1, inout $res) !== 13) echo "no\n";
if (apc_inc("ts", 5, inout $res) !== 18) echo "no\n";
if (apc_inc("ts", -3, inout $res) !== 15) echo "no\n";
echo "ok\n";
}
