<?php

require_once 'nowdoc.inc';

print <<<ENDOFHEREDOC
This is a heredoc test.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is another heredoc test.

ENDOFHEREDOC;

print "{$x}";

?>