<?hh


<<__EntryPoint>>
function main_no_exit() {
echo "Before error, there should be an 'After error'\n";
eval("function foo() { invalid }");
echo "After error\n";
}
