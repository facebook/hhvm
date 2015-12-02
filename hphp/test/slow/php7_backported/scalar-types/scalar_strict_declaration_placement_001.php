<?php

function takes_int(int $x) {
    global $errored;
    if ($errored) {
        echo "Failure!", PHP_EOL;
        $errored = FALSE;
    } else {
        echo "Success!", PHP_EOL;
    }
}

?>
<?php

declare(strict_types=1);
var_dump(takes_int(32));

?>
