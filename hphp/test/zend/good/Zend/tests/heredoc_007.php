<?hh
require_once 'nowdoc.inc';
<<__EntryPoint>> function main(): void {
include 'nowdoc_vars.inc';
print <<<ENDOFHEREDOC
This is heredoc test #s $a, {$b}, {$c['c']}, and {$d->d}.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is heredoc test #s $a, {$b}, {$c['c']}, and {$d->d}.

ENDOFHEREDOC;

print "{$x}";
}
