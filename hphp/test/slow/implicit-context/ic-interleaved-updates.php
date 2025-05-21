<?hh

function printMSIC(): void {
  $c = MemoSensitiveIntCtx::getContext();
  var_dump($c ? $c->getPayload() : null);
}

function printMAIC(): void {
  var_dump(MemoAgnosticIntCtx::getContext());
}

function s1(): void {
  printMSIC();
  printMAIC();
}

function a1(): void{
  printMAIC();
  MemoSensitiveIntCtx::start(new Base(6), s1<>);
  printMAIC();
  printMSIC(); // should throw since Memo sensitive IC is no longer set
}

<<__EntryPoint>>
function main(): mixed{
  include 'both-contexts.inc';
  MemoAgnosticIntCtx::start(5, a1<>);
  printMAIC(); // should return null
}
