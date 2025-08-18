<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class C<T> {
  public static function getMe(): this {
    throw new Exception();
  }

  public static function getTheThing(): T {
    throw new Exception();
  }

  public static function refThem(): void {
    $fptr = static::getMe<>;
    hh_expect<HH\FunctionRef<(readonly function<Tthis as C<Tc>, Tc>(): Tthis)>>(
      $fptr,
    );

    $gptr = static::getTheThing<>;
    hh_expect<HH\FunctionRef<(readonly function<Tc>(): Tc)>>($gptr);
  }
}

function refThem(): void {
  $fptr = C::getMe<>;
  hh_expect<HH\FunctionRef<(readonly function<Tthis as C<T>, T>(): Tthis)>>(
    $fptr,
  );

  $gptr = C::getTheThing<>;
  hh_expect<HH\FunctionRef<(readonly function<T>(): T)>>($gptr);
}
