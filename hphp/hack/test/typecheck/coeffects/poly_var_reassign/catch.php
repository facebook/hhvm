<?hh

abstract class V { abstract const ctx C; }

function catch_expr(V $v)[$v::C]: void {
  try {
    throw new Exception();
  } catch (Exception $v) {}
}
