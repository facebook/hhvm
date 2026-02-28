<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type StringData<T as string> =
  | mixed
  | nonnull
  | (function():void)
  | classname<mixed>
  | arraykey
  | string
  | ?T;

case type ResourceData<T as resource> =
  | mixed
  | nonnull
  | (function():void)
  | ?T;

case type BoolData<T as bool> =
  | mixed
  | nonnull
  | (function():void)
  | bool
  | ?T;

case type IntData<T as int> =
  | mixed
  | nonnull
  | (function():void)
  | num
  | arraykey
  | int
  | ?T;

case type FloatData<T as float> =
  | mixed
  | nonnull
  | (function():void)
  | num
  | float
  | ?T;

case type NullData<T as null> =
  | mixed
  | ?bool
  | void
  | null
  | T;
