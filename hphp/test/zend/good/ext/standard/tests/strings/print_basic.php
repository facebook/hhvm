<?hh

/* Prototype  : int print  ( string $arg  )
 * Description: Output a string
 * Source code: n/a, print is a language construct not an extension function
 * Test based on php.net manual example.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing print() : basic functionality ***\n";

echo "\n-- Iteration 1 --\n";
print("Hello World");

echo "\n-- Iteration 2 --\n";
print "print() also works without parentheses.";

echo "\n-- Iteration 3 --\n";
print "This spans
multiple lines. The newlines will be
output as well";

echo "\n-- Iteration 4 --\n";
print "This also spans\nmultiple lines. The newlines will be\noutput as well.";

echo "\n-- Iteration 5 --\n";
print "escaping characters is done \"Like this\".";

// You can use variables inside of a print statement
$foo = "foobar";
$bar = "barbaz";

echo "\n-- Iteration 6 --\n";
print "foo is $foo"; // foo is foobar

// You can also use arrays
$bar = dict["value" => "foo"];

echo "\n-- Iteration 7 --\n";
print "this is {$bar['value']} !"; // this is foo !

// Using single quotes will print the variable name, not the value
echo "\n-- Iteration 8 --\n";
print 'foo is $foo'; // foo is $foo

// If you are not using any other characters, you can just print variables
echo "\n-- Iteration 9 --\n";
print $foo;          // foobar

echo "\n-- Iteration 10 --\n";
$variable = "VARIABLE"; 
print <<<END
This uses the "here document" syntax to output
multiple lines with $variable interpolation. Note
that the here document terminator must appear on a
line with just a semicolon no extra whitespace!\n
END;
echo "===DONE===\n";
}
