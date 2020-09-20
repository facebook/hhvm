<?hh

<<__EntryPoint>> function main(): void {
print <<<'ENDOFNOWDOC'
This is a nowdoc test.

ENDOFNOWDOC;

$x = <<<'ENDOFNOWDOC'
This is another nowdoc test.
With another line in it.
ENDOFNOWDOC;

print "{$x}";
}
