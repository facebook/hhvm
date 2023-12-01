<?hh

function a2(): darray<int, string> {
  throw new Exception();
}

/* HH_FIXME[2071] */
function v0(): varray {
  return vec[];
}

function v1(): varray<int> {
  return vec[];
}

/* HH_FIXME[2071] */
function d0(): darray {
  return dict[];
}

function d2(): darray<int, string> {
  return dict[];
}

/* HH_FIXME[2071] */
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
