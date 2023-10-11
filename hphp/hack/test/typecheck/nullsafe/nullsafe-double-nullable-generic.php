<?hh

class Bar {
  public async function maybeFoo(): Awaitable<?Foo> {
    return null;
  }
  public static function getBar(): ?Bar {
    return new Bar();
  }
}

class Foo {}

class Policy<T> {
  public static function makePolicy<TDelegate>(
    vec<TDelegate> $foos,
    (function(): Awaitable<?TDelegate>) $f,
  ): Policy<TDelegate> {
    return new Policy();
  }
}

function needsFooPolicy(Policy<Foo> $p): void {}

function gen_test(): void {
  $policy = Policy::makePolicy(
    vec[new Foo()],
    async () ==> {
      $bar = Bar::getBar();
      return await $bar?->maybeFoo();
    },
  );
  // TDelegate should be Foo
  needsFooPolicy($policy);
}
