<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'simpli_hack')>>

function prompt(named string $message): string {
  return "Prompt: {$message}";
}

<<__SimpliHack(prompt(message = 'hello'))>>
function repro_single(): void {}

function greet(named string $first, named string $last): string {
  return "Hello {$first} {$last}";
}

<<__SimpliHack(greet(last = 'Doe', first = 'Jane'))>>
function repro_reordered(): void {}
//
