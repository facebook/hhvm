<?hh

function escapes_via_return(): vec<mixed> {
  return vec[dict[]];
}

function influenced_via_param(vec<dict<string, mixed>> $arg): void {
  $v = vec[dict[]];
  $v[] = $arg[0];
}
