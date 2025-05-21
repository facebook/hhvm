<?hh

// this test: recurse zig zag from memo agnostic to sensitive and back

function printAndReturnMSIC(): ?int {
  echo "MSIC: ";
  $c = MemoSensitiveIntCtx::getContext();
  var_dump ($c ? $c->getPayload() : 'NULL')."\n";
  return $c ? $c->getPayload() : null;
}

function printAndReturnMAIC(): ?int {
  echo "MAIC: ";
  var_dump(MemoAgnosticIntCtx::getContext());
  return MemoAgnosticIntCtx::getContext();
}

<<__Memoize>>
function test_memo(): void {
  echo "In test_memo...\n";
  printAndReturnMAIC(); // this will pass
  printAndReturnMSIC(); // this will throw
}

function s1(): void {
  echo "In memo sensitive...\n";
  printAndReturnMAIC();
  $x = printAndReturnMSIC();
  if ($x > 20) return;
  MemoAgnosticIntCtx::start($x+1, a1<>);
  var_dump($x);
}

function a1(): void{
  echo "In memo agnostic...\n";
  $x = printAndReturnMAIC();
  /*
   * Important: Even though we are in memo agnostic context, we can still
   * access memo sensitive context here. This is because we came here from
   * memo sensitive context, except the first call, when it prints "inaccessible".
   * If there exists a memo sensitive context at the time
   * we create a memo agnostic context, we create the memo agnostic
   * but return the pointer to the memo sensitive context, this is intended
   * behavior so we don't break existing memo sensitive functionality.
  */
  printAndReturnMSIC();
  /*
   * At the same time, if we go into a memoized function, the actieCtx will be
   * set to agnostic, and we will only be able to access memo agnostic context.
  */
  if ($x > 1) {
    test_memo();
  }
  if ($x > 20) return;
  MemoSensitiveIntCtx::start(new Base($x+1), s1<>);
  var_dump($x);
}

<<__EntryPoint>>
function main(): mixed{
  include 'both-contexts.inc';
  MemoAgnosticIntCtx::start(0, a1<>);
}
