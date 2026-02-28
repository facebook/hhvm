<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type NoDictToString = Stringish | dict<int, mixed>;

case type NoXHPBool = XHPChild | bool;
