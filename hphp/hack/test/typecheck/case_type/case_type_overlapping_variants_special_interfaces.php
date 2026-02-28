<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type ArrayData =
  | AnyArray<nothing, nothing>
  | vec_or_dict<nothing>
  | dict<nothing, nothing>
  | keyset<nothing>
  | vec<nothing>;

case type ContainerData =
  | Container<nothing>
  | dict<nothing, nothing>
  | keyset<nothing>
  | vec<nothing>;

case type KeyedContainerData =
  | KeyedContainer<nothing, nothing>
  | dict<nothing, nothing>
  | keyset<nothing>
  | vec<nothing>;

case type TraversableData =
  | Traversable<nothing>
  | dict<nothing, nothing>
  | keyset<nothing>
  | vec<nothing>;

case type KeyedTraversableData =
  | KeyedTraversable<nothing, nothing>
  | dict<nothing, nothing>
  | keyset<nothing>
  | vec<nothing>;

case type StringishData as Stringish =
  | Stringish
  | string;

case type XhpData as XHPChild =
  | XHPChild
  | string
  | int
  | float
  | vec<int>
  | keyset<int>
  | dict<int, int>;
