//// file1.php
<?hh // decl

async function foo() {
  yield 10;
}

//// file2.php
<?hh // strict

function bar(): void {
  // without the correct inference of foo as an async generator, this would
  // raise an error about an Awaitable being used dangerously
  foo();
}
