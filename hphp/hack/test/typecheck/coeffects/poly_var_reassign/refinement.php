<?hh

abstract class V { abstract const ctx C; }

// Expression ID is not updated in either case, only the type
function refine(V $v)[$v::C]: void {
  if ($v is int) {}
  $v as string;
}
