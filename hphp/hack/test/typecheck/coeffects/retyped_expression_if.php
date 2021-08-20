<?hh

function any(
  ?(function ()[_]: bool) $predicate = null,
)[ctx $predicate]: bool {
  throw new \Exception();
}

function f(): void {
  if (
    any(() ==> false) ||
    any(() ==> false)
  ) {
    throw new \Exception();
  }
}
