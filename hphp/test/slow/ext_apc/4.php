<?hh


// reproducing a memory leak (3/26/09)
<<__EntryPoint>>
function main_4() {
apc_add("apcdata", array("a" => "test", "b" => 1)); // MapVariant

$apcdata = __hhvm_intrinsics\apc_fetch_no_check("apcdata");
$c = $apcdata; // bump up ref count to make a MapVariant copy
$apcdata["b"] = 3; // problem
if ($apcdata !== array("a" => "test", "b" => 3)) echo "no\n";
unset($apcdata);

$apcdata = __hhvm_intrinsics\apc_fetch_no_check("apcdata");
$apcdata += array("b" => 4); // problem
if ($apcdata !== array("a" => "test", "b" => 1)) echo "no\n";
unset($apcdata);

$apcdata = __hhvm_intrinsics\apc_fetch_no_check(array("apcdata", "nah"));
if ($apcdata !== array("apcdata" => array("a" => "test", "b" => 1))) {
  echo "no\n";
}

echo "ok\n";
}
