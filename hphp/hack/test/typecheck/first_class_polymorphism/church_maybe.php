<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda','polymorphic_function_hints')>>

newtype Maybe<T> = (function<R>(R, (function(T):R)) : R);

class MaybeOps {

  public static function nothing<T>(): Maybe<T> {
    return (function<R>(R $n, (function(T):R) $_): R ==> $n);
  }

  public static function just<T>(T $x): Maybe<T> {
    return (function<R>(R $_, (function(T):R) $j): R ==> $j($x));
  }

  public static function map<TIn,TOut>(
    (function(TIn):TOut) $f,
    Maybe<TIn> $maybe,
  ): Maybe<TOut> {
    return (function<R>(R $n,(function(TOut):R) $j): R ==> {
      $g = $x ==> $j($f($x));
      return $maybe($n,$g);
    });
  }

  public static function bind<TIn,TOut>(
    (function(TIn):Maybe<TOut>) $f,
    Maybe<TIn> $maybe,
  ): Maybe<TOut> {
    return (function<R>(R $n,(function(TOut):R) $j): R ==> {
      $g = $x ==> $maybe($n, $y ==> $f($y)($n, $j));
      return $maybe($n, $g);
    });
  }

  public static function value<T>(Maybe<T> $maybe, T $default): T {
    return $maybe($default, $x ==> $x);
  }

  public static function isNothing<T>(Maybe<T> $maybe): bool {
    return $maybe(true, $_ ==> false);
  }

  public static function isJust<T>(Maybe<T> $maybe): bool {
    return $maybe(false, $_ ==> true);
  }

}
