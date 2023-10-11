<?hh

function get_map(): Map<string, int> {
  return Map {};
}

function testf(): void {
  $a = get_map();
  if ($a->contains("lol")) {
  }
  unset($a['hi']);
}
