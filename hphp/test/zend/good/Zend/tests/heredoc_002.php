<?hh

<<__EntryPoint>> function main(): void {
print b<<<ENDOFHEREDOC
This is a heredoc test.

ENDOFHEREDOC;

$x = b<<<ENDOFHEREDOC
This is another heredoc test.

ENDOFHEREDOC;

print "{$x}";
}
