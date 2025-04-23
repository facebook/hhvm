<?hh
<<file: __EnableUnstableFeatures('simpli_hack')>>

function prompt3(string $txt): string {
  if ($txt === '') {
    return $txt;
  }
  return 'None';
}

function prompt2(string $txt): string {
  return prompt3("More {$txt}");
}

function prompt(string $txt): string {
  return prompt2($txt.$txt);
}

<<__SimpliHack(prompt('Test'))>>
function f(): void {
}
