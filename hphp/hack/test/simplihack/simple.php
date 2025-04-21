<?hh

function prompt(string $msg): string {
  return "This is a prompt for {$msg}";
}

<<__SimpliHack(prompt('the function `f`'))>>
function f(): void {
}
