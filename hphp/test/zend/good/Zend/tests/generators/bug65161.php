<?hh

function autoload() {
    foo();
}

function testGenerator() {
    new SyntaxError('param');
    yield;
}

<<__EntryPoint>>
function main_entry(): void {
  spl_autoload_register('autoload');

  foreach (testGenerator() as $i);
}
