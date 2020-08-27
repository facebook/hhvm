<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

function expect<T>(T $_): void {}

class C {}

function testit<T>(T $_): void {
  expect<int>(:@message);
  expect<float>(:@message);
  expect<T>(:@message);
  expect<C>(:@message);
}
