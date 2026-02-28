<?hh

<<__EntryPoint>>
function main_closure_export() :mixed{
$c = function() { print "42"; };
var_dump(var_export($c, true));
}
