<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class CNamedEnforced<T> {
  public function __construct(T $value, named int $label) {}
}

function test(): void {
  new CNamedEnforced(label = 1, 'value');
//                           ^ enforcement-at-caret
}
