<?hh


<<__EntryPoint>>
function main_5() {
apc_store("ts", "TestString");
apc_store("ta", array("a" => 1, "b" => 2));
apc_delete("ts");
apc_delete("ta");
if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== false) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ta") !== false) echo "no\n";

if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== false) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ta") !== false) echo "no\n";

apc_store("ts", "TestString");
apc_store("ta", array("a" => 1, "b" => 2));
if (apc_delete(array("ts", "ta")) !== array()) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== false) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ta") !== false) echo "no\n";
echo "ok\n";
}
