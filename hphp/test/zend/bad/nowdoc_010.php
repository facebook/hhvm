<?php

require_once 'nowdoc.inc';
$fooledYou = '';

print <<<'ENDOFNOWDOC'
{$fooledYou}ENDOFNOWDOC{$fooledYou}
ENDOFNOWDOC{$fooledYou}
{$fooledYou}ENDOFNOWDOC

ENDOFNOWDOC;

$x = <<<'ENDOFNOWDOC'
{$fooledYou}ENDOFNOWDOC{$fooledYou}
ENDOFNOWDOC{$fooledYou}
{$fooledYou}ENDOFNOWDOC

ENDOFNOWDOC;

print "{$x}";

?>