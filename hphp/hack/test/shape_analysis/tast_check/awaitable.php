<?hh

async function f(): Awaitable<dict<string, mixed>> {
  // Shouldn't produce a result because it escapes local definition
  return dict[];
}
