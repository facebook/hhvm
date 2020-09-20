<?hh

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] Sadly */
type UNSAFE_TYPE = UNSAFE_TYPE_HH_FIXME_;

abstract final class HH_FIXME {
  const type TEMPORARY_MISSING_TYPE_MARKER = UNSAFE_TYPE;
}

function f(
  darray<
    HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER,
    HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER,
  > $x,
): void {
  g($x);
}
function g(darray<int, string> $_): void {}
