<?php

require_once 'nowdoc.inc';

print b<<<ENDOFHEREDOC
This is a heredoc test.

ENDOFHEREDOC;

$x = b<<<ENDOFHEREDOC
This is another heredoc test.

ENDOFHEREDOC;

print "{$x}";

?>