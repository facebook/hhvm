<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

type OptionalNamed = (function(optional named int $value): void);
type NamedOptional = (function(named optional int $value): void);
