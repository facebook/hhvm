<?hh

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] Sadly */
type UNSAFE_TYPE = UNSAFE_TYPE_HH_FIXME_;

abstract final class HH_FIXME {
  const type TEMPORARY_MISSING_TYPE_MARKER = UNSAFE_TYPE;
}

function f1(
  varray_or_darray<
    HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER,
    HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER,
  > $x,
): void {
  g1($x);
}
function g1(varray_or_darray<string> $_): void {}

function f2(
  varray_or_darray<
    HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER,
    HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER,
  > $x,
): void {
  g2($x);
}
function g2(varray_or_darray<int, bool> $_): void {}

function f3(
  varray_or_darray<
    HH_FIXME::TEMPORARY_MISSING_TYPE_MARKER,
  > $x,
): void {
  g3($x);
}
function g3(varray_or_darray<bool> $_): void {}
