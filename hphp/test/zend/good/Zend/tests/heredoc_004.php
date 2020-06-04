<?hh
<<__EntryPoint>> function main(): void {
require_once 'nowdoc.inc';
include 'nowdoc_vars.inc';
print <<<ENDOFHEREDOC
This is heredoc test #{$a}.

ENDOFHEREDOC;

$x = <<<ENDOFHEREDOC
This is heredoc test #{$b}.

ENDOFHEREDOC;

print "{$x}";
}
