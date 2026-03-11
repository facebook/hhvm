<?hh
<<file:__EnableUnstableFeatures('case_types')>>

// Recursive case type bounds that previously caused an infinite loop during
// type checking. The cycle was: localize -> intersect_list -> sub_type -> localize
// when simplifying the intersection in the upper bound constraint.
case type MyEtDemoAutoLiftable as int, MyEtDemoOpType = string;

case type MyEtDemoOpType = vec<MyEtDemoAutoLiftable>;
