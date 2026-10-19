<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

type DuplicateNamedParameter =
  (function(named int $value, optional named string $value): void);
//
