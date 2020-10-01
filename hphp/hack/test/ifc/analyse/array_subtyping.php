<?hh

class C {}

function dictToDictPrim(dict<string,bool> $dict): dict<arraykey,bool> {
  return $dict;
}

function dictToDictClass(dict<string,C> $dict): dict<arraykey,C> {
  return $dict;
}

function dictToKTClass(dict<string,C> $dict): KeyedTraversable<arraykey,C> {
  return $dict;
}

function vecToVecPrim(vec<bool> $vec): vec<bool> {
  return $vec;
}

function vecToVecClass(vec<C> $vec): vec<C> {
  return $vec;
}

function vecToKTClass(vec<C> $vec): KeyedTraversable<int,C> {
  return $vec;
}

function keysetToKeysetPrim(keyset<int> $keyset): keyset<int> {
  return $keyset;
}

function keysetToKTPrim(keyset<int> $keyset): KeyedTraversable<int,int> {
  return $keyset;
}
