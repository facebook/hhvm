<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// A `named` parameter in a function type must have a name.

function bad1((function(named int): void) $_): void {} // Error

function bad2((function(named int $x, named string): void) $_): void {} // Error

function bad3((function(optional named int): void) $_): void {} // Error

// OK: named param with a name.
function ok1((function(named int $x): void) $_): void {}

// OK: named variadic (trailing `...` in place of `$name`).
function ok2((function(named int...): void) $_): void {}
