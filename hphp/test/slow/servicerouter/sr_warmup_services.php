<?hh

// Exercises the sr_warmup_services async builtin end to end: it must resolve to
// a dict keyed by exactly the requested tiers, each mapped to a bool. Warmup
// outcomes depend on live SMC (and no-such-tier folds to success), so only the
// shape of the result is asserted here, not the individual bool values.
<<__EntryPoint>>
async function main(): Awaitable<void> {
  $tiers = keyset["token.service", "servicerouter"];
  $results = await sr_warmup_services($tiers);

  var_dump($results is dict<_, _>);

  $all_bool = true;
  foreach ($results as $ok) {
    $all_bool = $all_bool && ($ok is bool);
  }
  var_dump($all_bool);

  $result_keys = vec[];
  foreach ($results as $tier => $_) {
    $result_keys[] = $tier;
  }
  sort(inout $result_keys);
  var_dump($result_keys);
}
