<?hh


<<__EntryPoint>>
function main_bad_strpos_result() {
var_dump(iconv_strpos('11112', '112'));
var_dump(iconv_strpos('12122', '122'));
}
