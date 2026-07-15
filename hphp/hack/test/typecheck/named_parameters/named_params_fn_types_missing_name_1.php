<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

// A `named` parameter in a function type must have a name.
function bad1((function(named int): void) $_): void {} // Error
