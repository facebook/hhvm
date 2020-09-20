<?hh

<<__EntryPoint>>
function main_10() {
apc_store("ts", "TestString");
if (apc_exists("ts") !== true) echo "no\n";
if (apc_exists("TestString") !== false) echo "no\n";
if (apc_exists(varray["ts", "TestString"] !== varray["ts"])) echo "no\n";
echo "ok\n";
}
