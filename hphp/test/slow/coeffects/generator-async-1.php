<?hh

async function f()[rx] :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  yield 2;
}

async function pure($gen)[]:Awaitable<mixed>{
  foreach ($gen await as $e) {
    echo $e . "\n";
  }
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await pure(f());
}
