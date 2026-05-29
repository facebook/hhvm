<?hh

<<__EntryPoint>> function main(): void {
print <<<'ENDOFNOWDOC'
This is a nowdoc test.

ENDOFNOWDOC;

$x = <<<'ENDOFNOWDOC'
This is another nowdoc test.

ENDOFNOWDOC;

print "{$x}";
}
