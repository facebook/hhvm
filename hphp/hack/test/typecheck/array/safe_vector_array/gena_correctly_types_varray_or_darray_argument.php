<?hh

async function gen_nullable_int(): Awaitable<?int> {
  return null;
}

function gen_varray_or_darray_of_awaitable_of_nullable_int(
): varray_or_darray<Awaitable<?int>> {
  return varray[gen_nullable_int()];
}

async function gen_int_varray_or_darray(): Awaitable<varray_or_darray<int>> {
  return await gena(gen_varray_or_darray_of_awaitable_of_nullable_int());
}
