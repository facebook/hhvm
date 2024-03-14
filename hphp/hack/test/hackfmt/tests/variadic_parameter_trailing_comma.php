<?hh

function myPrintfWithANameThatIsSoLongItBreaksTheDeclarationUp(string $format, ...) {
  // ...
}

function someOtherFunctionWithALongNameCausingLineBreaksToBeAdded(mixed ...$args) {
  // ...
}

function testVariadicsInFunctionType((function(int,string...):bool) $f): void { }
function testVariadicsInFunctionType2((function(mixed...):string) $f): void { }

class C {
  public static function fuzzy<
    Tk as arraykey,
    T,
    TViewerContext as IViewerContextBase,
  >(
    int $chunk_size,
    ?int $seed,
    Tk $default_key,
    IChunqFieldGetterBase<T, mixed, EntQLQueryParams<TViewerContext>> ...$getters
  ): IChunqFieldGetterBase<T, int, EntQLQueryParams<TViewerContext>> {
  }
}
