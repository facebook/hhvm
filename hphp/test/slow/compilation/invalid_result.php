<?hh

function test() :AsyncGenerator<mixed,mixed,void>{
  list($a, $b) = yield wait_forva(result(1), result(2));
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
