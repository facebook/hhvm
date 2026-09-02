<?hh
// error, produced in naming phase
function rejects_optional_positional_before_required_positional(
  (function(optional int, bool): void) $_,
): void {}
// error, produced in naming phase
function rejects_optional_positional_before_named_then_required_positional(
  (function(optional int, named int $y, bool): void) $_,
): void {}
