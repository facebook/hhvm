<?hh
<<file:__EnableUnstableFeatures('case_types')>>

module some.mod;

internal case type CT1 = int;

public case type CT2 = int;

<<SomeAttr>>
case type CT3 = int;
