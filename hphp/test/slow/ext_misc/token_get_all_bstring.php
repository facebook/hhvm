<?php

$code = '<?php b"$foo";';
foreach (token_get_all($code) as $token) {
    if (is_string($token)) {
        echo $token, "\n";
    } else {
        echo token_name($token[0]) . ": ", $token[1], "\n";
    }
}
