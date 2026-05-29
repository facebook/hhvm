<?hh

<<__EntryPoint>> function main(): void {
print <<<ENDOFHEREDOC
This is a heredoc test.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is another heredoc test.

ENDOFHEREDOC;

print "{$x}";
}
