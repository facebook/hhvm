<?hh

<<__EntryPoint>> function main(): void {
  $a = vec[null, dict['cat' => 'meow', 'dog' => 'woof']];
  var_dump($a[0]['unused']);
  var_dump($a[1]['dog']);
  try { var_dump($a[2]['unused']); } catch (Exception $e) { echo $e->getMessage()."\n"; }

  apc_store('widget', $a);
  unset($a);
  $a = __hhvm_intrinsics\apc_fetch_no_check('widget');
  var_dump($a[0]['unused']);
  var_dump($a[1]['cat']);
  try { var_dump($a[2]['unused']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
