<?hh

final class ReifiedBox<reify T> {}

case type RecursiveThroughBox =
  | ReifiedBox<RecursiveThroughBox>
  | ReifiedBox<int>
  | int;
