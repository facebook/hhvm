<?hh


// clearstatcache do nothing, but there should no warnnings
<<__EntryPoint>>
function main_clearstatcache() :mixed{
var_dump(clearstatcache());
var_dump(clearstatcache(true));
var_dump(clearstatcache(false, "test.php"));
}
