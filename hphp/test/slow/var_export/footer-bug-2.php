<?hh

<<__EntryPoint>>
function main_footer_bug_2() :mixed{
$obj = new stdClass();
$obj->prop = Vector{};
var_export($obj);
echo "\nDone\n";
}
