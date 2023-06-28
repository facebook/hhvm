<?hh

<<__EntryPoint>>
function main_9() :mixed{
apc_store("ts", 12);
apc_cas("ts", 12, 15);
if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== 15) echo "no\n";
apc_cas("ts", 12, 18);
if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== 15) echo "no\n";
echo "ok\n";
}
