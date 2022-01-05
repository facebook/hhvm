<?hh

async function foo(): Awaitable<void> {
  await async {
    //  ^ hover-at-caret
    return 1;
  };
}
