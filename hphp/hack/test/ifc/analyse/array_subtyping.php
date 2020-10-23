<?hh

class C {}

<<__InferFlows>>
function dictToDictPrim(dict<string,bool> $dict): dict<arraykey,bool> {
  return $dict;
}

<<__InferFlows>>
function dictToDictClass(dict<string,C> $dict): dict<arraykey,C> {
  return $dict;
}

<<__InferFlows>>
function dictToKTClass(dict<string,C> $dict): KeyedTraversable<arraykey,C> {
  return $dict;
}

<<__InferFlows>>
function vecToVecPrim(vec<bool> $vec): vec<bool> {
  return $vec;
}

<<__InferFlows>>
function vecToVecClass(vec<C> $vec): vec<C> {
  return $vec;
}

<<__InferFlows>>
function vecToKTClass(vec<C> $vec): KeyedTraversable<int,C> {
  return $vec;
}

<<__InferFlows>>
function keysetToKeysetPrim(keyset<int> $keyset): keyset<int> {
  return $keyset;
}

<<__InferFlows>>
function keysetToKTPrim(keyset<int> $keyset): KeyedTraversable<int,int> {
  return $keyset;
}
