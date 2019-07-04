<?hh
require_once 'nowdoc.inc';
<<__EntryPoint>> function main(): void {
include 'nowdoc_vars.inc';
print <<<'ENDOFNOWDOC'
This is nowdoc test #$a.

ENDOFNOWDOC;

$x = <<<'ENDOFNOWDOC'
This is nowdoc test #$b.

ENDOFNOWDOC;

print "{$x}";
}
