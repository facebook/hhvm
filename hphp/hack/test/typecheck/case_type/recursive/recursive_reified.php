<?hh
<<file: __EnableUnstableFeatures('recursive_case_types')>>

final class ReifiedBox<reify T> {}

case type RecursiveThroughBox =
  | ReifiedBox<RecursiveThroughBox>
  | ReifiedBox<int>
  | int;
