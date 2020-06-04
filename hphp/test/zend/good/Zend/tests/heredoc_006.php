<?hh
<<__EntryPoint>> function main(): void {
require_once 'nowdoc.inc';
include 'nowdoc_vars.inc';
print <<<ENDOFHEREDOC
This is heredoc test #s {$a}, {$b}, {$c['c']}, and {$d->d}.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is heredoc test #s {$a}, {$b}, {$c['c']}, and {$d->d}.

ENDOFHEREDOC;

print "{$x}";
}
