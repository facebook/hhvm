<?hh

case type CT1 = vec<int>;

case type CT2 as arraykey, num = int;

case type CT3 as nonnull = string | int | bool;

case type CT4 = | string | int | bool;

<<SomeAttr, OtherAttr>>
case type CT5<+Tout, -Tin, T> = null;
