<?hh

<<__EntryPoint>>
function test_header_entrypoint() {
header('Content-Disposition: attachment; filename='.$_GET['test_string']);
var_dump("OK");
}
