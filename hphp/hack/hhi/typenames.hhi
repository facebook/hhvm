<?hh

namespace HH {
  newtype FormatString<T> as string = string;

  <<__NoAutoDynamic>>
  newtype FunctionRef<T> as T = T;
}
