<?hh


<<__EntryPoint>>
function main_6() :mixed{
apc_store("ts", "TestString");
apc_store("ta", dict["a" => 1, "b" => 2]);

apc_clear_cache();
if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== false) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ta") !== false) echo "no\n";

if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== false) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ta") !== false) echo "no\n";

echo "ok\n";
}
