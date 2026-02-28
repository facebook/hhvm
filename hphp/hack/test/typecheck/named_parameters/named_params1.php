<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

function test_named_params(int $positional1, named int $named1, named int $named2_opt = 3): void {}
