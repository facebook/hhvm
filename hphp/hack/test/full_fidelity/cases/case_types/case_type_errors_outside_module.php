<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type CT = C::T;

internal case type CT2 = int;

public case type CT3 = int | C::T;
