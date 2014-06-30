<?php
function utf32_utf8($k) {
	if ($k < 0x80) {
		$retval = pack('C', $k);
	} else if ($k < 0x800) {
		$retval = pack('C2', 
            0xc0 | ($k >> 6),
            0x80 | ($k & 0x3f));
	} else if ($k < 0x10000) {
        $retval = pack('C3',
            0xe0 | ($k >> 12),
            0x80 | (($k >> 6) & 0x3f),
            0x80 | ($k & 0x3f));
	} else if ($k < 0x200000) {
        $retval = pack('C4',
            0xf0 | ($k >> 18),
            0x80 | (($k >> 12) & 0x3f),
            0x80 | (($k >> 6) & 0x3f),
            0x80 | ($k & 0x3f));
	} else if ($k < 0x4000000) {
        $retval = pack('C5',
            0xf8 | ($k >> 24),
            0x80 | (($k >> 18) & 0x3f),
            0x80 | (($k >> 12) & 0x3f),
            0x80 | (($k >> 6) & 0x3f),
            0x80 | ($k & 0x3f));
	} else {
        $retval = pack('C6',
            0xfc | ($k >> 30),
            0x80 | (($k >> 24) & 0x3f),
            0x80 | (($k >> 18) & 0x3f),
            0x80 | (($k >> 12) & 0x3f),
            0x80 | (($k >> 6) & 0x3f),
            0x80 | ($k & 0x3f));
	}
	return $retval;
}

$table = get_html_translation_table(HTML_ENTITIES, ENT_QUOTES, 'UTF-8');

for ($i = 0; $i < 0x2710; $i++) {
    if ($i >= 0xd800 && $i < 0xe000)
        continue;
    $str = utf32_utf8($i);
	if (isset($table[$str])) {
		printf("%s\tU+%05X\n", $table[$str], $i);
		unset($table[$str]);
	}
}

if (!empty($table)) {
	echo "Not matched entities: ";
	var_dump($table);
}

?>