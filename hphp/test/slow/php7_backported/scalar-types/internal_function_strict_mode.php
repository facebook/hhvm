<?php
declare(strict_types=1);

require 'fix_exceptions.inc';

echo "*** Trying Ord With Integer" . PHP_EOL;
try {
    var_dump(ord(1));
} catch (TypeError $e) {
    echo "*** Caught " . $e->getMessage() . PHP_EOL;
}

echo "*** Trying Array Map With Invalid Callback" . PHP_EOL;
try {
    array_map([null, "bar"], []);
} catch (TypeError $e) {
    echo "*** Caught " . $e->getMessage() . PHP_EOL;
}

// strlen currently does it's own hand-rolled type-checking because we emit a
// bytecode for it. This needs to be cleaned up in a follow up before the
// following will work
//
//echo "*** Trying Strlen With Float" . PHP_EOL;
//try {
//    var_dump(strlen(1.5));
//} catch (TypeError $e) {
//    echo "*** Caught " . $e->getMessage() . PHP_EOL;
//}

?>
