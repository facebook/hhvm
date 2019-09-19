<?hh


<<__EntryPoint>>
function main_3() {
$complexMap = array(
  "f" => "facebook",
  "a" => array("b" => 1,
               "c" => array("d", "e")),
  "f" => array(1,2,3),
  "h" => "hello",
);

apc_store("complexMap", $complexMap);
apc_store("ts", "TestString");
apc_store("ta", array("a" => 1, "b" => 2));
apc_store("ts", "NewValue");
apc_store("ta", array("newelement"));
if (apc_store($complexMap) != array()) echo "no\n";

if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== "NewValue") echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ta") !== array("newelement")) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("h") !== "hello") echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("complexMap") !== $complexMap) echo "no\n";

if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== "NewValue") echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ta") !== array("newelement")) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("f") !== array(1,2,3)) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("complexMap") !== $complexMap) echo "no\n";

// Make sure it doesn't change the shared value.
$complexMapFetched = __hhvm_intrinsics\apc_fetch_no_check("complexMap");
if (!isset($complexMapFetched['a'])) echo "no\n";
$complexMapFetched['q'] = 0;
if (!isset($complexMapFetched['q'])) echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("complexMap") !== $complexMap) echo "no\n";

$tsFetched = __hhvm_intrinsics\apc_fetch_no_check("ts");
if ($tsFetched !== "NewValue") echo "no\n";
$sharedString = $tsFetched;
$tsFetched[0] = "M";
if ($tsFetched !== "MewValue") echo "no\n";
if ($sharedString !== "NewValue") echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("ts") !== "NewValue") echo "no\n";
if (__hhvm_intrinsics\apc_fetch_no_check("a") !== array("b" => 1,
                             "c" => array("d", "e"))) echo "no\n";

echo "ok\n";
}
