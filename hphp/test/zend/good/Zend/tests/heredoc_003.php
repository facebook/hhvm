<?hh
<<__EntryPoint>> function main(): void {
require_once 'nowdoc.inc';
require_once 'nowdoc_vars.inc';
$a = a();
$b = b();
print <<<ENDOFHEREDOC
This is heredoc test #$a.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is heredoc test #$b.

ENDOFHEREDOC;

print "{$x}";
}
