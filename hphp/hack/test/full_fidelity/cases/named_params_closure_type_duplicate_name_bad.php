<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

type DuplicateAdjacent =
  (function(named int $value, optional named string $value): void);

type DuplicateSeparated =
  (function(named int $value, bool, named string $value): void);

type Distinct =
  (function(named int $first, optional named string $second): void);

type FixedAndNamedVariadic =
  (function(named int $value, named string...): void);
//
