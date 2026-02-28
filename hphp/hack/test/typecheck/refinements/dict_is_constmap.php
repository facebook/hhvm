<?hh

function repro(
  HH\Lib\Ref<dict<int, dict<int, int>>> $rdictofdict,
): void {
  $edges = $rdictofdict->value;
  if ($edges is ConstMap<_, _>) {
    $edges = dict($edges);
  }
  foreach ($edges as $k => $v) {
    foreach ($v as $_) {
    }
  }
}
