<?hh
// Occupies the single request thread; ?ms tunes how long.
<<__EntryPoint>>
function main_timeout_race_slow(): void {
  $get = HH\global_get('_GET');
  usleep(((int)($get['ms'] ?? 2900)) * 1000);
  echo "slow\n";
}
