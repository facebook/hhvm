<?hh

// 'now' and '' in a fixed-offset zone ('-09:00', '+05:30', ...) must resolve
// to the current instant, with a local time that agrees with that instant.
// Named zones and UTC are controls. The checks hold at any time of day, so the
// output is stable.

function check_now(string $zone, string $input): void {
  $tz = new DateTimeZone($zone);
  $before = time();
  $d = new DateTimeImmutable($input, $tz);
  $after = time();
  $ts = $d->getTimestamp();

  // The object is the current instant.
  if ($ts < $before) {
    $instant = 'off by '.($ts - $before).'s';
  } else if ($ts > $after) {
    $instant = 'off by +'.($ts - $after).'s';
  } else {
    $instant = 'ok';
  }

  // Its local time is that instant in the zone.
  $expected_local = (new DateTimeImmutable('@'.$ts))->setTimezone($tz);
  $skew = (new DateTimeImmutable($d->format('Y-m-d H:i:s'), new DateTimeZone('UTC')))
    ->getTimestamp() -
    (new DateTimeImmutable($expected_local->format('Y-m-d H:i:s'), new DateTimeZone('UTC')))
    ->getTimestamp();
  $local = $skew === 0 ? 'ok' : 'off by '.($skew > 0 ? '+' : '').$skew.'s';

  printf("%-17s %-5s instant: %s, local time: %s\n", $zone, var_export($input, true), $instant, $local);
}

<<__EntryPoint>>
function main_now_with_fixed_offset_timezone(): void {
  date_default_timezone_set('UTC');
  foreach (vec['-09:00', '+05:30', '+14:00', '-12:00', 'UTC', 'America/Anchorage', 'Etc/GMT+9'] as $zone) {
    foreach (vec['now', ''] as $input) {
      check_now($zone, $input);
    }
  }
}
