<?hh


<<__EntryPoint>>
function main_exit() {
print "Before error\n";
eval('blah();');
print "After error, this should not show!\n";
}
