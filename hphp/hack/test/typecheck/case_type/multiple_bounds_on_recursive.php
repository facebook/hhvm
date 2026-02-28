<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type CTSub as mixed, CTSuper = int;

case type CTSuper = int | vec<CTSub>;
