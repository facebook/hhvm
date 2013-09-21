<?php

require_once 'nowdoc.inc';
$fooledYou = '';

print <<<ENDOFHEREDOC
{$fooledYou}ENDOFHEREDOC{$fooledYou}
ENDOFHEREDOC{$fooledYou}
{$fooledYou}ENDOFHEREDOC

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
{$fooledYou}ENDOFHEREDOC{$fooledYou}
ENDOFHEREDOC{$fooledYou}
{$fooledYou}ENDOFHEREDOC

ENDOFHEREDOC;

print "{$x}";

?>