<?hh

function not_a_builtin(int $_): void {}

$result = [];
// Calls to builtins from Hack files are not currently strict; we will revisit
// this if the HHIs become more strict, or as PHP builtins are replaced with
// Hack ones.
//
// First argument should be a string, not null.
parse_str(null, &$result);
var_dump($result);

// But, non-builtins should be strict in PHP7 mode - this should error:
not_a_builtin('123');
