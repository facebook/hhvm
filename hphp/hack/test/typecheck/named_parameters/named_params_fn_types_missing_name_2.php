<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// A `named` parameter in a function type must have a name, even with `optional`.
function bad3((function(optional named int): void) $_): void {} // Error
