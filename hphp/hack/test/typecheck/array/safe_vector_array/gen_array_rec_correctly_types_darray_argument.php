<?hh

async function gen_nullable_int(): Awaitable<?int> {
  return null;
}

async function gen_string_to_int_darray(): Awaitable<darray<string, int>> {
  return await gen_array_rec(darray['key' => gen_nullable_int()]);
}
