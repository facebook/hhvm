<?php
function decode($json) {
    var_dump(json_decode($json));
    echo ((json_last_error() !== 0) ? 'ERROR' : 'SUCCESS') . PHP_EOL;
}

// Only lowercase should work
decode('true');
decode('True');
decode('[true]');
decode('[True]');
echo PHP_EOL;

decode('false');
decode('False');
decode('[false]');
decode('[False]');
echo PHP_EOL;

decode('null');
decode('Null');
decode('[null]');
decode('[Null]');
echo PHP_EOL;

echo "Done\n";
