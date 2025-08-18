<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

function expecting<T>(T $_): void {}

class C {
  public static function getMe(): this {
    throw new Exception();
  }

  public static function refThem(): void {
    $fptr = C::getMe<>;
    expecting<HH\FunctionRef<(readonly function<Tthis as C>(): Tthis)>>($fptr);

    $gptr = static::getMe<>;
    expecting<HH\FunctionRef<(readonly function<Tthis as C>(): Tthis)>>($gptr);
  }
}
