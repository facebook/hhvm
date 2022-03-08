<?hh

function get_map(): Map<string, mixed> {
  invariant_violation('haha');
}

function unset_map(): void {
  unset(get_map());
  unset(get_map()['oops!']);
}
