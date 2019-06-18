<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

/* HH_FIXME[4110] */
function array_create_set_from_values<T as arraykey>(
   Traversable<T> $arr,
 ): darray<T, T> {
}

function maybe_varray_map<Tv1, Tv2>(
  (function(Tv1): Tv2) $value_func,
  Traversable<Tv1> $traversable,
): varray<Tv2> {
  return varray[];
}

class C {
  public function getID():int {
    return 3;
  }
}

function darray_map<Tk as arraykey, Tv1, Tv2>(
  (function(Tv1): Tv2) $value_func,
  KeyedTraversable<Tk, Tv1> $traversable,
): darray<Tk, Tv2> {
  return darray[];
}
async function genProfileInfos(): Awaitable<Vector<C>> {
    return Vector{};
}
/* HH_FIXME[4110] */
async function genm<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<Map<Tk, Tv>> {
}
async function testit(Map<arraykey, Vector<C>> $m, varray<int> $user_ids):Awaitable<void> {
  $m = await genm(
      darray_map(
        $id ==> genProfileInfos(),
        $user_ids,
      ),
    );

  $result = darray[];
   foreach ($m as $id => $profiles) {
      $result[$id] = array_create_set_from_values(
        darray(maybe_varray_map($x1 ==> $x1->getID(), $profiles)),
      );
   }
}
