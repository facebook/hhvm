<?hh // partial

function a2(): darray<int, string> {
  throw new Exception();
}

function v0(): varray {
  return varray[];
}

function v1(): varray<int> {
  return varray[];
}

function d0(): darray {
  return darray[];
}

function d2(): darray<int, string> {
  return darray[];
}

function vd0(): varray_or_darray {
  throw new Exception();
}

function vd1(): varray_or_darray<int> {
  throw new Exception();
}

function vd2(): varray_or_darray<int, string> {
  throw new Exception();
}

function vd3(): varray_or_darray<int, string, bool> {
  throw new Exception();
}
