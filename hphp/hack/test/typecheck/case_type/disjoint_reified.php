<?hh

type MyAlias = float;

final class ReifiedBox<reify T> {}

case type CTGood = ReifiedBox<int> | ReifiedBox<string>;

case type CTBad = ReifiedBox<MyAlias> | ReifiedBox<float>;
