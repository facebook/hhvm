<?php

require_once 'nowdoc.inc';

print b<<<'ENDOFNOWDOC'
This is a nowdoc test.

ENDOFNOWDOC;

$x = b<<<'ENDOFNOWDOC'
This is another nowdoc test.

ENDOFNOWDOC;

print "{$x}";

?>