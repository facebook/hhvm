<?hh

// Expression ID is not updated in either case, only the type
function refine(vec<int> $v)[$v::C]: void {
  if ($v is int) {}
  $v as string;
}
