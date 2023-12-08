<?hh

namespace HH {
  newtype FormatString<T> = string;

  <<__NoAutoDynamic>>
  newtype FunctionRef<T> as T = T;
}
