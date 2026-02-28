<?hh

// special message about annotation here
function voids1() : mixed {
  return;
}

// special message about annotation here
function voids2() : mixed {
}

// only general type mismatch error here
function voids3() : int {
}
