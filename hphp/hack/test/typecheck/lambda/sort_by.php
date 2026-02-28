<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class TimePeriod {
  public function getScore(): float {
    return 2.0;
  }
}

function sort_by<Tv, Ts>(
  Traversable<Tv> $t,
  (function(Tv): Ts) $s,
  ?(function(Ts, Ts): int) $c = null,
): vec<Tv> {
  throw new Exception("A");
}

async function genUnits(vec<TimePeriod> $units): Awaitable<vec<TimePeriod>> {
  return
    sort_by($units, (TimePeriod $u) ==> $u->getScore(), ($a, $b) ==> $b - $a);
}
