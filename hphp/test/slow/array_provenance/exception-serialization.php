<?hh

function get_exception() {
  try {
    1 / 0;
  } catch (Exception $e) {
    return $e;
  }
}

function try_serialize($label, $value) {
  print("\n==============================================================\n");
  print("try_serialize($label):\n");
  print('json_encode:  '.json_encode($value)."\n");
  print('serialize:    '.serialize($value)."\n");
  print('fb_serialize: '.json_encode(fb_serialize($value))."\n");
}

<<__EntryPoint>>
function main() {
  $exn = get_exception();
  try_serialize('$exn', $exn);
  try_serialize('$exn->getTrace()', $exn->getTrace());
  try_serialize('array_mark_legacy_recursive($exn->getTrace())',
                HH\array_mark_legacy_recursive($exn->getTrace()));
  try_serialize('array_cast($exn)',
                HH\array_mark_legacy(HH\object_prop_array($exn)));
}
