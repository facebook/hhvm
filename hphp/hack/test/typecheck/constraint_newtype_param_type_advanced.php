<?hh

newtype X<T as arraykey> = T;
type Y<T> = X<T>;

function bar(Y<Y<Y<Y<int>>>> $_): void {}
function make1<T>(T $x): Y<Y<Y<T>>> {
  return $x;
}
function make2<T as arraykey>(T $x): Y<Y<Y<Y<T>>>> {
  return $x;
}
function make3(int $x): Y<Y<Y<Y<int>>>> {
  return $x;
}
