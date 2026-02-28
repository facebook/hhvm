<?hh

function a2(): darray<int, string> {
  throw new Exception();
}

function v1(): varray<int> {
  return vec[];
}

function d2(): darray<int, string> {
  return dict[];
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
