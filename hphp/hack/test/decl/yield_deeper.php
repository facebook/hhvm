<?hh

function f(): Generator<bool, string, int> {
  $x = yield true => 'a';
}

function g(): void {
  $x = async function() {
    yield 0;
  };
}

function h(): void {
  $x = () ==> {
    yield 0;
  };
}

async function i(): Awaitable<void> {
  await async {
    yield 0;
    yield 1;
  };
}

async function j(): AsyncKeyedIterator<string, int> {
  if (true) {
    yield 'a' => 1;
  } else {
    yield 'b' => 2;
  }
  if (false) {
    yield 'c' => 3;
  } else {
    yield 'd' => 4;
  }
}
