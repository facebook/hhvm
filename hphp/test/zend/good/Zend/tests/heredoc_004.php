<?hh
require_once 'nowdoc.inc';
<<__EntryPoint>> function main(): void {
include 'nowdoc_vars.inc';
print <<<ENDOFHEREDOC
This is heredoc test #{$a}.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is heredoc test #{$b}.

ENDOFHEREDOC;

print "{$x}";
}
