<?hh


// reproducing a memory leak (3/26/09)
<<__EntryPoint>>
function main_4() :mixed{
apc_add("apcdata", darray["a" => "test", "b" => 1]); // MapVariant

$apcdata = __hhvm_intrinsics\apc_fetch_no_check("apcdata");
$c = $apcdata; // bump up ref count to make a MapVariant copy
$apcdata["b"] = 3; // problem
if ($apcdata !== darray["a" => "test", "b" => 3]) echo "no\n";
unset($apcdata);

$apcdata = __hhvm_intrinsics\apc_fetch_no_check("apcdata");
$apcdata["b"] = 4;
if ($apcdata !== darray["a" => "test", "b" => 4]) echo "no\n";
unset($apcdata);

$apcdata = __hhvm_intrinsics\apc_fetch_no_check(varray["apcdata", "nah"]);
if ($apcdata !== darray["apcdata" => darray["a" => "test", "b" => 1]]) {
  echo "no\n";
}

echo "ok\n";
}
