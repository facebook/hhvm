<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type ArrayCollection = vec<int> | Vector<int>;

case type KeyedArrayCollection = dict<int, mixed> | Map<int, mixed>;

case type ExtendArrayCollection = ArrayCollection | int;
