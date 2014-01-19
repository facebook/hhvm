<?php
function _bin2hex($val) {
    return is_string($val) ? bin2hex($val): $val;
}

echo "UTF-8: basic tests\n";
var_dump(_bin2hex(htmlentities("\xc1\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xc2\x80", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xc2\x00", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xc2\xc0", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xce\x91", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xce\xb1", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xdf\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe0\xa0\x80", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe0\x9f\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe0\x9f\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe0\x1f\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe0\x9f\x3f", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe0\x1f\x3f", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe2\x99\xa5", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xef\xbf\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xef\xff\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xef\xbf\xff", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf0\x8f\xbf\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf0\x90\x80\x80", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf7\xbf\xbf\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf7\x3f\xbf\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf7\xbf\x3f\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf7\xbf\xbf\x3f", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf7\xff\xbf\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf7\xbf\xff\xbf", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf7\xbf\xbf\xff", ENT_QUOTES, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf8\x88\x80\x80\x80", ENT_QUOTES, "UTF-8")));

echo "--\n";
echo "UTF-8: with ENT_IGNORE\n";
var_dump(_bin2hex(htmlentities("\xc0\xa0\xc2\x80", ENT_QUOTES | ENT_IGNORE, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xe0\x80\x80\xe0\xa0\x80", ENT_QUOTES | ENT_IGNORE, "UTF-8")));
var_dump(_bin2hex(htmlentities("\xf0\x80\x80\x80\xf0\x90\x80\x80", ENT_QUOTES | ENT_IGNORE, "UTF-8")));

echo "--\n";
echo "UTF-8: alternative (invalid) UTF-8 sequence / surrogate pairs\n";
var_dump(_bin2hex(htmlspecialchars("\xc0\xa6", ENT_QUOTES, 'UTF-8')));
var_dump(_bin2hex(htmlspecialchars("\xe0\x80\xa6", ENT_QUOTES, 'UTF-8')));
var_dump(_bin2hex(htmlspecialchars("\xf0\x80\x80\xa6", ENT_QUOTES, 'UTF-8')));
var_dump(_bin2hex(htmlspecialchars("\xec\xbf\xbf", ENT_QUOTES, 'UTF-8')));
var_dump(_bin2hex(htmlspecialchars("\xed\xa0\x80", ENT_QUOTES, 'UTF-8')));
var_dump(_bin2hex(htmlspecialchars("\xed\xbf\xbf", ENT_QUOTES, 'UTF-8')));
var_dump(_bin2hex(htmlspecialchars("\xee\x80\x80", ENT_QUOTES, 'UTF-8')));

echo "--\n";
echo "Shift_JIS: non-lead byte >= 0x80\n";
var_dump(_bin2hex(htmlspecialchars("\x80", ENT_QUOTES, 'Shift_JIS')));
foreach (array_map('chr', range(0xa0, 0xdf)) as $c) {
    var_dump(_bin2hex(htmlspecialchars($c, ENT_QUOTES, 'Shift_JIS')));
}
var_dump(_bin2hex(htmlspecialchars("\xfd", ENT_QUOTES, 'Shift_JIS')));
var_dump(_bin2hex(htmlspecialchars("\xfe", ENT_QUOTES, 'Shift_JIS')));
var_dump(_bin2hex(htmlspecialchars("\xff", ENT_QUOTES, 'Shift_JIS')));

echo "--\n";
echo "Shift_JIS: incomplete / invalid multibyte sequences\n";
foreach (array_map('chr', array_merge(range(0x81, 0x9f), range(0xe0, 0xfc))) as $c) {
    var_dump(_bin2hex(htmlspecialchars("$c", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\x3f", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\x40", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\x7e", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\x7f", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\x80", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\xfc", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\xfd", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\xfe", ENT_QUOTES, 'Shift_JIS')));
    var_dump(_bin2hex(htmlspecialchars("$c\xff", ENT_QUOTES, 'Shift_JIS')));
}

echo "--\n";
echo "EUC-JP: non-lead byte >= 0x80\n";
foreach (array_map('chr', array_merge(range(0x80, 0x8d), range(0x90, 0x9f))) as $c) {
    var_dump(_bin2hex(htmlspecialchars($c, ENT_QUOTES, 'EUC-JP')));
}
var_dump(_bin2hex(htmlspecialchars("\xff", ENT_QUOTES, 'EUC-JP')));

// EUC-JP: control codes that are virtually lead bytes
var_dump(_bin2hex(htmlspecialchars("\x8e", ENT_QUOTES, 'EUC-JP')));
var_dump(_bin2hex(htmlspecialchars("\x8f", ENT_QUOTES, 'EUC-JP')));
var_dump(_bin2hex(htmlspecialchars("\x8e\xa1", ENT_QUOTES, 'EUC-JP')));
var_dump(_bin2hex(htmlspecialchars("\x8f\xa1", ENT_QUOTES, 'EUC-JP')));
var_dump(_bin2hex(htmlspecialchars("\x8e\xa1\xa3", ENT_QUOTES, 'EUC-JP')));
var_dump(_bin2hex(htmlspecialchars("\x8f\xa1\xa3", ENT_QUOTES, 'EUC-JP')));

echo "--\n";
echo "EUC-JP: incomplete / invalid multibyte sequences\n";
foreach (array_map('chr', array_merge(range(0xa1, 0xfe))) as $c) {
    var_dump(_bin2hex(htmlspecialchars("$c", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("$c\x26", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("$c\x80", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("$c\xa0", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("$c\xa1", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("$c\xfe", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("$c\xff", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8e$c", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8e$c\x26", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8e$c\x80", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8e$c\xa0", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8e$c\xa1", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8e$c\xfe", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8e$c\xff", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8f$c", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8f$c\x26", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8f$c\x80", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8f$c\xa0", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8f$c\xa1", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8f$c\xfe", ENT_QUOTES, 'EUC-JP')));
    var_dump(_bin2hex(htmlspecialchars("\x8f$c\xff", ENT_QUOTES, 'EUC-JP')));
}

echo "--\n";
echo "BIG5: non-lead byte >= 0x80\n";
var_dump(_bin2hex(htmlspecialchars("\x80", ENT_QUOTES, 'BIG5')));
var_dump(_bin2hex(htmlspecialchars("\xff", ENT_QUOTES, 'BIG5')));

echo "--\n";
echo "BIG5: incomplete / invalid multibyte sequences\n";
foreach (array_map('chr', range(0x81, 0xfe)) as $c) {
    var_dump(_bin2hex(htmlspecialchars("$c", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\x3f", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\x40", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\x7e", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\x7f", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\x80", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\xa0", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\xa1", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\xfe", ENT_QUOTES, 'BIG5')));
    var_dump(_bin2hex(htmlspecialchars("$c\xff", ENT_QUOTES, 'BIG5')));
}
?>