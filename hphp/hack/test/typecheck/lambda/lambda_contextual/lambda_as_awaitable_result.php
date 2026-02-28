<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function bar(): void {}
  public function get(): int {
    return 3;
  }
}

type Fun = (function(C): Awaitable<void>);
type StringFun = (function(C): Awaitable<string>);
async function genFoo(): Awaitable<Fun> {
  return async ($c) ==> {
    $c->bar();
  };
}

function foo(): Fun {
  return async ($c) ==> {
    $c->bar();
  };
}
async function genBoo(): Awaitable<StringFun> {
  return async ($c) ==> {
    $c->bar();
    return "a";
  };
}

async function genv<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<Vector<Tv>> {
  return Vector {};
}

async function genNullable(arraykey $key): Awaitable<?C> {
  return null;
}
async function moreComplexExample(varray<int> $property_ids): Awaitable<void> {
  $sss = array_map(
    async ($property_id) ==> {
      $property = await genNullable($property_id);
      return $property !== null ? $property->get() : null;
    },
    $property_ids,
  );
  $event_source_ids = await genv($sss);
}

async function genAwaitableNullable(): Awaitable<?C> {
  // So we test Awaitable<C> <: Awaitable<?C>
  // We should be expecting ?C
  return new C();
}

async function genAwaitableNullable2(): Awaitable<?C> {
  // So we test Awaitable<?_> <: Awaitable<?C>
  // We should be expecting ?C
  return null;
}

async function genVec(): Awaitable<vec<arraykey>> {
  return vec[
    "a",
    2.3,
    true,
  ];
}
