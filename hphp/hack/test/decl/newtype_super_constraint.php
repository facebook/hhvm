<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>
newtype X as arraykey super string = string;
newtype Y super num = mixed;
