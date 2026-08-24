<?hh

async function pull<ToutKey as arraykey, ToutValue>(
  (function(string, int): Awaitable<ToutValue>) $_value,
  (function(string, int): Awaitable<ToutKey>) $_key,
): Awaitable<dict<ToutKey, ToutValue>> {
  return dict[];
}

function merge<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $_first,
  KeyedContainer<Tk, Tv> $_second,
): dict<Tk, Tv> {
  return dict[];
}

async function get_params(): Awaitable<dict<string, string>> {
  return dict[];
}

async function repro(): Awaitable<dict<string, string>> {
  $by_key = await pull(
    async ($key, $_index) ==> await get_params(),
    async ($key, $_index) ==> $key,
  );
  $result = $by_key['initial'];

  // Keep these as straight-line calls: a loop typechecks its body only once.
  $result = merge($result, $by_key['0']);
  $result = merge($result, $by_key['1']);
  $result = merge($result, $by_key['2']);
  $result = merge($result, $by_key['3']);
  $result = merge($result, $by_key['4']);
  $result = merge($result, $by_key['5']);
  $result = merge($result, $by_key['6']);
  $result = merge($result, $by_key['7']);
  $result = merge($result, $by_key['8']);
  $result = merge($result, $by_key['9']);
  $result = merge($result, $by_key['10']);
  $result = merge($result, $by_key['11']);
  $result = merge($result, $by_key['12']);
  $result = merge($result, $by_key['13']);
  $result = merge($result, $by_key['14']);
  $result = merge($result, $by_key['15']);
  $result = merge($result, $by_key['16']);
  $result = merge($result, $by_key['17']);
  $result = merge($result, $by_key['18']);
  $result = merge($result, $by_key['19']);
  $result = merge($result, $by_key['20']);
  $result = merge($result, $by_key['21']);
  $result = merge($result, $by_key['22']);
  $result = merge($result, $by_key['23']);
  $result = merge($result, $by_key['24']);
  $result = merge($result, $by_key['25']);
  $result = merge($result, $by_key['26']);
  $result = merge($result, $by_key['27']);
  $result = merge($result, $by_key['28']);
  $result = merge($result, $by_key['29']);
  $result = merge($result, $by_key['30']);
  $result = merge($result, $by_key['31']);
  $result = merge($result, $by_key['32']);
  $result = merge($result, $by_key['33']);
  $result = merge($result, $by_key['34']);
  $result = merge($result, $by_key['35']);
  $result = merge($result, $by_key['36']);
  $result = merge($result, $by_key['37']);
  $result = merge($result, $by_key['38']);
  $result = merge($result, $by_key['39']);
  $result = merge($result, $by_key['40']);
  $result = merge($result, $by_key['41']);
  $result = merge($result, $by_key['42']);
  $result = merge($result, $by_key['43']);
  $result = merge($result, $by_key['44']);
  $result = merge($result, $by_key['45']);
  $result = merge($result, $by_key['46']);
  $result = merge($result, $by_key['47']);
  $result = merge($result, $by_key['48']);
  $result = merge($result, $by_key['49']);
  $result = merge($result, $by_key['50']);
  $result = merge($result, $by_key['51']);
  $result = merge($result, $by_key['52']);
  $result = merge($result, $by_key['53']);
  $result = merge($result, $by_key['54']);
  $result = merge($result, $by_key['55']);
  $result = merge($result, $by_key['56']);
  $result = merge($result, $by_key['57']);
  $result = merge($result, $by_key['58']);
  $result = merge($result, $by_key['59']);
  $result = merge($result, $by_key['60']);
  $result = merge($result, $by_key['61']);
  $result = merge($result, $by_key['62']);
  $result = merge($result, $by_key['63']);
  $result = merge($result, $by_key['64']);
  $result = merge($result, $by_key['65']);
  $result = merge($result, $by_key['66']);
  $result = merge($result, $by_key['67']);
  $result = merge($result, $by_key['68']);
  $result = merge($result, $by_key['69']);
  $result = merge($result, $by_key['70']);
  $result = merge($result, $by_key['71']);
  $result = merge($result, $by_key['72']);
  $result = merge($result, $by_key['73']);
  $result = merge($result, $by_key['74']);
  $result = merge($result, $by_key['75']);
  $result = merge($result, $by_key['76']);
  $result = merge($result, $by_key['77']);

  return $result;
}
