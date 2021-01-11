<?hh

function catch_expr(vec<int> $v)[$v::C]: void {
  try {
    throw new Exception();
  } catch (Exception $v) {}
}
