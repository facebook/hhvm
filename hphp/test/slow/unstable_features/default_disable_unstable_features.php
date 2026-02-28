<?hh

// The default configuration should be that unstable features are disabled,
// thus using the attribute with an unstable feature should result in a fatal error

<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

function f(): (int|bool) { return 0; }
