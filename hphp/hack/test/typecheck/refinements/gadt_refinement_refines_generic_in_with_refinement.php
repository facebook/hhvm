<?hh

interface IHasTypeConst {
  abstract const type T;
}

interface ITypeConstInt extends IHasTypeConst {
  const type T = int;
}

function mytest<Tf>(IHasTypeConst with { type T = Tf } $x): Tf {
  if ($x is ITypeConstInt) {
    return 0;
  }
  throw new Exception();
}
