<?hh


<<__EntryPoint>>
function main_mb_decode_numericentity() :mixed{
$convmap = vec[0x0, 0x2FFFF, 0, 0xFFFF];
var_dump(mb_decode_numericentity("&#8217;&#7936;&#226;", $convmap, "UTF-8") ===
     "\xe2\x80\x99\xe1\xbc\x80\xc3\xa2");

var_dump(mb_decode_numericentity("&#102;&#111;&#111;", $convmap) === "foo");

$convmap = vec[0x0, 0x2FFFF, 0, 0xFFFF];
var_dump(mb_encode_numericentity("\xe2\x80\x99\xe1\xbc\x80\xc3\xa2",
                                 $convmap, "UTF-8") ===
   "&#8217;&#7936;&#226;");
var_dump(mb_encode_numericentity("\xe2\x80\x99\xe1\xbc\x80\xc3\xa2",
                                 $convmap, "UTF-8", true) ===
   "&#x2019;&#x1F00;&#xE2;");
var_dump(mb_encode_numericentity("foo", $convmap) === "&#102;&#111;&#111;");
}
