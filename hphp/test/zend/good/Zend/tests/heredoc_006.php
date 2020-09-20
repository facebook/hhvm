<?hh
<<__EntryPoint>> function main(): void {
require_once 'nowdoc.inc';
require_once 'nowdoc_vars.inc';
$a = a();
$b = b();
$c = c();
$d = d();
print <<<ENDOFHEREDOC
This is heredoc test #s {$a}, {$b}, {$c['c']}, and {$d->d}.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is heredoc test #s {$a}, {$b}, {$c['c']}, and {$d->d}.

ENDOFHEREDOC;

print "{$x}";
}
