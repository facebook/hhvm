<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class DictError {
  const type TAst = mixed;

  public static function makeTree<TInfer>(
    ?ExprPos $pos,
    shape(
      // Based on a user report of an error that didn't make much sense
      'splices' => dict<string, Spliceable<DictError, DictError::TAst, TInfer>>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>, ?'type' => TInfer,
    ) $metadata,
    (function(DictError): DictError::TAst) $ast,
  ): ExprTree<DictError, DictError::TAst, TInfer> {
    throw new Exception();
  }

  public static function voidType(): ExampleVoid {
    throw new Exception();
  }

  public static function lambdaType<T>(T $_): ExampleFunction<T> {
    throw new Exception();
  }

  public function visitLocal(?ExprPos $_, string $_): DictError::TAst {
    throw new Exception();
  }

  public function visitLambda(
    ?ExprPos $_,
    vec<string> $_args,
    vec<DictError::TAst> $_body,
  ): DictError::TAst {
    throw new Exception();
  }

  public function visitAssign(
    ?ExprPos $_,
    DictError::TAst $_,
    DictError::TAst $_,
  ): DictError::TAst {
    throw new Exception();
  }

  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    Spliceable<DictError, DictError::TAst, T> $_,
  ): DictError::TAst {
    throw new Exception();
  }
}

function test(ExampleInt $x): void {
  DictError`() ==> {
    // Error about dict values being incompatible should refer to the Visitor hint
    $y = ${$x};
  }`;
}
