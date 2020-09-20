<?hh

<<__EntryPoint>>
function main_utf8_encode() {
utf8_encode(str_repeat('x', 1<<30));
print "Done\n";
}
