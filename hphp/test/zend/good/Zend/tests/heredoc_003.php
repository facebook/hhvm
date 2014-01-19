<?php

require_once 'nowdoc.inc';

print <<<ENDOFHEREDOC
This is heredoc test #$a.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is heredoc test #$b.

ENDOFHEREDOC;

print "{$x}";

?>