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

declare(strict_types=1) {
    function strict_calls_takes_int() {
        takes_int(1.0); // should fail, strict mode
    }

    class StrictTakesIntCaller {
        public function call() {
            takes_int(1.0); // should fail, strict mode
        }
    }
}

declare(strict_types=0) {
    function explicit_weak_calls_takes_int() {
        takes_int(1.0); // should succeed, weak mode
    }

    class ExplicitWeakTakesIntCaller {
        public function call() {
            takes_int(1.0); // should succeed, weak mode
        }
    }
}


?>
