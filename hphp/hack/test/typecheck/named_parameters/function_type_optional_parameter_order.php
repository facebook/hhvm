<?hh
// should be accepted, but is not
function accepts_optional_named_before_required_named_and_positional(
  (function(optional named int $x, named int $y, bool): void) $_,
): void {}
// should be accepted, but is not
function accepts_optional_positional_before_required_named(
  (function(optional int, named int $y): void) $_,
): void {}
