<?hh
<<file: __EnableUnstableFeatures('open_tuples')>>
namespace HH {
  newtype FormatString<T> as TypedFormatString<T, (mixed...)> = string;

  newtype TypedFormatString<T, Targs as (mixed...)> as string = string;

  <<__NoAutoDynamic>>
  newtype FunctionRef<T> as T = T;
}
