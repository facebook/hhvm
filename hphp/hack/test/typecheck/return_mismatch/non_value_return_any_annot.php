<?hh

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] */
type UNSAFE_TYPE = UNSAFE_TYPE_HH_FIXME_;

abstract final class HH_FIXME {
  const type TEMPORARY_MISSING_TYPE_MARKER = UNSAFE_TYPE;
}

//okay
function test1() : HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER {
  return;
}

//okay
async function test2() : Awaitable<HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER> {
  return;
}
