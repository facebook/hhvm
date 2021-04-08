<?hh

// The default configuration should be that unstable features are disabled,
// thus using the attribute with an unstable feature should result in a fatal error

<<file:__EnableUnstableFeatures('class_level_where')>>

interface I where this as I {}
