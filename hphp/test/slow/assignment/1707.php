<?hh

function g($key, $old, $new, $s = false) :mixed{
  $diff = dict[];
  if ($old !== $new) {
    if ($s) {
      $old = f($old, true);
      $new = f($new, true);
    }
    $diff['old'] = dict[];
    $diff['old'][$key] = $old;
    $diff['new'] = dict[];
    $diff['new'][$key] = $new;
  }
  return $diff;
}
function f($a0, $a1) :mixed{
  return 'should_be_modified';
}

<<__EntryPoint>>
function main_1707() :mixed{
var_dump(g('key', 'old', 'new', true));
}
