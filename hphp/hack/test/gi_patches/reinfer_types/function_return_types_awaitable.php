<?hh //partial

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] Please add the expected type parameters */
type UNSAFE_TYPE_HH_FIXME = UNSAFE_TYPE_HH_FIXME_;

abstract final class HH_FIXME {
  const type MISSING_TYPE = UNSAFE_TYPE_HH_FIXME;
}

async function fa(int $x): Awaitable<mixed> {
  return 1;
}

async function fb(int $x): Awaitable<dynamic> {
  return "string";
}

async function fc(int $x): Awaitable<UNSAFE_TYPE_HH_FIXME> {
  return 1.0;
}

async function fd(int $x): Awaitable<HH_FIXME::MISSING_TYPE>  {
  return 1;
}
