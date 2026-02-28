<?hh

interface MyExpression<+T as ?ClientTypeNonNull> {}

interface ClientTypeNonNull extends MyExpression<?ClientTypeNonNull> {
}

interface ClientLambda<+T> {
  public function __unwrap()[]: T;
}

abstract class ResponseStruct implements ClientTypeNonNull {
  public ClientTypeNonNull $info;
}

function getResult2(): ?ResponseStruct {
  throw new Exception();
}

function makeTree<TInfer as ?ClientTypeNonNull>(
  (function(): TInfer) $metadata,
): MyExpression<TInfer> {
  throw new Exception();
}

function asNonnull<T as ClientTypeNonNull>(): (function(?T): T) {
  throw new Exception();
}

function lambdaType<T>(T $_)[]: ClientLambda<T> {
  throw new Exception();
}

function g(): void {
  $f =
    (shape(...) $fv = shape()) ==> {
      $l = () ==> {
        $r = false ? asNonnull()(getResult2())->info : null;
        return $r;
      };
      $x = lambdaType($l)->__unwrap()();
      return $x;
  };
  makeTree($f);
}
