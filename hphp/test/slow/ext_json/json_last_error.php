<?php
// A valid json string
$json[] = '{"Organization": "PHP Documentation Team"}';

// An invalid json string which will cause an syntax
// error, in this case we used ' instead of " for quotation
$json[] = "{'Organization': 'PHP Documentation Team'}";

// A valid json string
$json[] = "";

// A valid json string
$json[] = "{}";

foreach ($json as $string) {
    echo 'Decoding: ' . $string . ' - ';
    json_decode($string);
    echo json_last_error_msg();
    echo "\n";
}

