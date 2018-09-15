<?hh

async function gen_nullable_int(): Awaitable<?int> {
  return null;
}

async function gen_int_varray(): Awaitable<varray<int>> {
  return await gena(varray[gen_nullable_int()]);
}
