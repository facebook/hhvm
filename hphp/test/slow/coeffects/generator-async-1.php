<?hh

async function f()[rx] {
  yield 1;
  yield 2;
}

async function pure($gen)[]{
  foreach ($gen await as $e) {
    echo $e . "\n";
  }
}

<<__EntryPoint>>
async function main() {
  await pure(f());
}
