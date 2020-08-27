<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

function expect_key(arraykey $_): void {}
function expect_string(string $_): void {}

class C {
  enum E {
    case int val;
    :@I(
      val = 42
    );
  }
}

function expect0(C:@E $_): void {}
function expect1<TP as C:@E>(TP $_): void {}

function testit(): void {
  expect_key(:@message);
  expect_string(:@message);
  expect0(:@I);
  expect1(:@I);
}
