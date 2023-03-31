<?hh

async function awaitable_awaitable_good1(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    async $x ==> await awaitable_awaitable_good1($x), // should not trigger rule
  );
}

async function awaitable_awaitable_good2(int $x): Awaitable<void> {
  // annotated inline lambda, should not trigger rule
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    async ($x): Awaitable<Awaitable<void>> ==> awaitable_awaitable_good2($x),
  );
}

async function awaitable_awaitable_good3(int $x): Awaitable<void> {
  $_ = await HH\Lib\Vec\map_async(
    vec[],
    // no nested Awaitable here
    $x ==> {
      return awaitable_awaitable_good3($x);
    },
  );
}

async function gen_map<Ta, Tb>(
  Traversable<Ta> $_,
  (function(Ta): Awaitable<Tb>) $_,
): Awaitable<Vector<Tb>> {
  return Vector {};
}

async function awaitable_awaitable_good4(int $x): Awaitable<void> {
  // compound lambda with Awaitable<T>, should not trigger rule
  $candidate_ids = vec[1, 2, 3, 4, 5];
  $_ = await gen_map(
    $candidate_ids,
    async ($candidate_id) ==> {
      return Pair {$candidate_id, 0.0};
    },
  );
}

async function awaitable_awaitable_good5(int $x): Awaitable<void> {
  // annotated compound lambda, should not trigger rule
  $f = async (int $x): Awaitable<Awaitable<void>> ==> {
    $y = 1;
    if ($x === 2) {
      return awaitable_awaitable_good5($x);
    } else {
      return awaitable_awaitable_good5($x + $y);
    }
  };
  $_ = await $f(0);
}
