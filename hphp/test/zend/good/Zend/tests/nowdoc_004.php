<?hh
<<__EntryPoint>> function main(): void {
require_once 'nowdoc.inc';
include 'nowdoc_vars.inc';
print <<<'ENDOFNOWDOC'
This is nowdoc test #{$a}.

ENDOFNOWDOC;

$x = <<<'ENDOFNOWDOC'
This is nowdoc test #{$b}.

ENDOFNOWDOC;

print "{$x}";
}
