<?hh

<<__EntryPoint>> function main(): void {
$x = <<<'ENDOFNOWDOC'
This is a nowdoc test.
NOTREALLYEND;
Another line
NOTENDEITHER;
ENDOFNOWDOCWILLBESOON
Now let's finish it
ENDOFNOWDOC;
print "{$x}\n";
}
