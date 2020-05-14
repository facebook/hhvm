<?hh

// This will run, but it's not doing what the user expects. We never
// pass arguments to entry points.
<<__EntryPoint>>
function bad_variadic(mixed ...$_): void {}
