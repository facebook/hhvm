<?hh

<<__EntryPoint>>
function main_bad_utf8() :mixed{
var_dump(utf8_decode(addslashes("\xf0\xc0\xc0\xa7 or 1=1-- -")));

$tests = vec[
  "\x41\xC2\x3E\x42",
  "\xE3\x80\x22",
  "\x41\x98\xBA\x42\xE2\x98\x43\xE2\x98\xBA\xE2\x98",
];
foreach ($tests as $t) {
  var_dump(bin2hex(utf8_decode($t)));
}
echo "Done.\n";
}
