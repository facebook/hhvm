<?hh
<<file: __EnableUnstableFeatures('pipe_await')>>


async function g(int $i): Awaitable<int> {
  return $i;
}

function h(int $i): int {
  return $i;
}

async function f(): Awaitable<void> {
  h(await g(1) + (await g(2) |> await g($$)));
  h((await g(2) |> await g($$)) + await g(1));
  h(await g(1) + (await g(await g(2))));
  h((await g(await g(2))) + await g(1));

  concurrent {
    await g(1);
    await g(1) |> await g($$);
  }

  concurrent {
    await g(1) |> await g($$);
    await g(1);
  }

  concurrent {
    await g(1);
    await g(await g(1));
  }

  concurrent {
    await g(await g(1));
    await g(1);
  }

  concurrent {
    h((await g(await g(2))) + await g(1));
    await g(1);
  }

  concurrent {
    await g(1);
    await async { await g(1) |> await g($$); };
    await async { (await g(1) + (await g(1) |> await g($$))); };
  }
}
