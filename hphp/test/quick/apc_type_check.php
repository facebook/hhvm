<?hh

<<__EntryPoint>>
function main() : mixed {
  $exists = false;
  $key = "type_check_ub";
  $id = "apc_type_check_run_id";
  apc_fetch($id, inout $exists);
  if (!$exists) {
    apc_add($id, 1);
  }
  var_dump($id." exists?: ".($exists ? "true" : "false"));

  $id = $exists ? 2 : 1;
  include(__DIR__."/apc_type_check_def$id.inc");
  if ($id === 1) {
    $a = new A<int>(1);
    apc_store($key, $a);
    var_dump("Stored to apc");
    var_dump($a);
  } else {
    $success = false;
    $z = apc_fetch($key, inout $success);
    var_dump("Done with fetch");
    var_dump($z);
  }
}
