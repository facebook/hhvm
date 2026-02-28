<?hh

<<__EntryPoint>>
async function main() {
  $counts = get_active_worker_counts();

  // Check that all expected keys are present
  var_dump(HH\Lib\C\contains_key($counts, "pagelet_workers"));
  var_dump(HH\Lib\C\contains_key($counts, "xbox_workers"));
  var_dump(HH\Lib\C\contains_key($counts, "http_workers"));
  var_dump(HH\Lib\C\contains_key($counts, "cli_workers"));

  // Check that all values are integers
  var_dump(HH\Lib\C\every($counts, $count ==> $count is int));

  // Check that all values are non-negative
  var_dump(HH\Lib\C\every($counts, $count ==> $count >= 0));
}
