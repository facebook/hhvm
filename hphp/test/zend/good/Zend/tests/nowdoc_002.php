<?hh

<<__EntryPoint>> function main(): void {
print b<<<'ENDOFNOWDOC'
This is a nowdoc test.

ENDOFNOWDOC;

$x = b<<<'ENDOFNOWDOC'
This is another nowdoc test.

ENDOFNOWDOC;

print "{$x}";
}
