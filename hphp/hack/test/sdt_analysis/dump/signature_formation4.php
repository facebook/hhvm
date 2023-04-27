<?hh

// This function doesn't have to be SDT but the current conservative check
// thinks it is.
function f<T>(T $t): T { return $t; }
