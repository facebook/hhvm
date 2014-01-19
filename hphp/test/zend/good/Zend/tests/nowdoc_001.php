<?php

require_once 'nowdoc.inc';

print <<<'ENDOFNOWDOC'
This is a nowdoc test.

ENDOFNOWDOC;

$x = <<<'ENDOFNOWDOC'
This is another nowdoc test.
With another line in it.
ENDOFNOWDOC;

print "{$x}";

?>