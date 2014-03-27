<?php
$e = ezc_try_call( function() { ezc_throw_std(); } );
print get_class($e) . ": " . $e->getMessage() . "\n";
