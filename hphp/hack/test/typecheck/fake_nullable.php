<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function darray_filter_falsy<Tk as arraykey, Tv as nonnull>(
  KeyedTraversable<Tk, ?Tv> $traversable,
): darray<Tk, Tv> {
  return darray[];
}
async function genak<Tk as arraykey, Tv>(
  (function (Tk): Awaitable<Tv>) $gen,
  Traversable<Tk> $keys,
): Awaitable<darray<Tk, Tv>> {
  return darray[];
}
interface I { }

function tany() {
  return null;
}
async function gennull():Awaitable<?I> {
  return null;
}
class C {

  private darray $fld = darray[];
  public async function foo() {
    $this->fld = await genak<_,_>(
      async (int $id) : Awaitable<?I> ==> await gennull(),
      tany(),
    );
    $x = darray_filter_falsy<int,_>($this->fld);
  }
}
