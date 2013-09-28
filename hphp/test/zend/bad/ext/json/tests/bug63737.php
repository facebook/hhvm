<?php
function decode($json) {
    $x = json_decode($json);
    var_dump($x);
    $x = json_decode($json, false, 512, JSON_BIGINT_AS_STRING);
    var_dump($x);
}

decode('123456789012345678901234567890');
decode('-123456789012345678901234567890');

// This shouldn't affect floats, but let's check that.
decode('123456789012345678901234567890.1');
decode('-123456789012345678901234567890.1');

echo "Done\n";
?>