<?hh

<<__EntryPoint>>
function main_utf8_encode() :mixed{
utf8_encode(str_repeat("\xFF", 1<<30));
print "Done\n";
}
