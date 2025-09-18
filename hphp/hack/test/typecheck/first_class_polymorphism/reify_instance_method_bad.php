<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class Wibble<reify T> {
  public function wobble(): string {
    return "Wobble";
  }
}

function whoop(): void {
  $f = meth_caller(Wibble::class, 'wobble');
}
