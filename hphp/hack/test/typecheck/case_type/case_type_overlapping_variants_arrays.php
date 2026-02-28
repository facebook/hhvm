<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type DictData =
  | mixed
  | nonnull
  | (function():void)
  | TypeStructure<int>
  | shape()
  | dict<int, int>;

case type KeysetData =
  | mixed
  | nonnull
  | (function():void)
  | keyset<int>;

case type VecData =
  | mixed
  | nonnull
  | (function():void)
  | (int, int)
  | vec<int>;
