<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type CT1<+T1 as arraykey, -T2, T> as nonnull = keyset<T1> | (function(T2): T);

case type CT2 as arraykey, num = int;

case type CT3 = string | bool;
