<?hh

<<__EntryPoint>>
function main(): void {
  // Real tier — should be tracked (slot 1 of 2)
  sr_get_connection2("zkcron", null, dict[], false);

  // Fake tier — getChannel() throws, should NOT be tracked
  sr_get_connection2("fake.nonexistent.tier", null, dict[], false);

  // Duplicate — should be deduplicated by the set, still 1 tier tracked
  sr_get_connection2("zkcron", null, dict[], false);

  // Second real tier — should be tracked (slot 2 of 2)
  sr_get_connection2("zkcron.global", null, dict[], false);

  // Third real tier — should be rejected by cap (maxTiers=2, set already full)
  sr_get_connection2("ods_router.global", null, dict[], false);

  // Retrieve tracked tiers from the jumpstart profile
  $tiers = extension_warmup_data("servicerouter");

  // Sort for deterministic output
  if ($tiers is vec<_>) {
    sort(inout $tiers);
  }
  var_dump($tiers);
}
