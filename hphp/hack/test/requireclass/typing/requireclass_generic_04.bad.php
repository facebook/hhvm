<?hh

<<file:__EnableUnstableFeatures('require_class')>>

interface I {}

trait T1<T> {
  require class T;
}

trait T2<T> {
  require extends T;
}
