<?hh
// Method call on non-object using FCallObjMethod
$foo = 12;
$func = "bar";
$foo->$func();
echo "Hi\n";
