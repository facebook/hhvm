<?php
foreach (array("Shift_JIS", "CP932") as $enc) {
    for ($a = 0; $a < 256; $a++) {
        var_dump(mb_check_encoding("\x81".pack("c", $a), $enc));
    }
}
?>