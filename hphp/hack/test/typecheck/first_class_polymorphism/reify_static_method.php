<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class Wibble<reify T> {
  public static function wobble(): string {
    return "Wobble";
  }
}

function whoop(): void {
  $f = Wibble::wobble<>;
  hh_expect<HH\FunctionRef<(readonly function(): string)>>($f);
}
