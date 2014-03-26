<?php
$e = ezc_try_call( function() { ezc_throw_nonstd(); } );
print get_class($e) . ": " . $e->getMessage() . "\n";
