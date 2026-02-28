//// ~root/a.php
// Content of file a.php

//// b.php
<?hh
<<file:__EnableUnstableFeatures('simpli_hack')>>

function prompt(string $file)[HH\SimpliHack]: string {
  $content = HH\SimpliHack\file('a.php');
  return "file: {$file}
  content: {$content}";
}

<<__SimpliHack(prompt('a.php'))>>
function f(): void {
}
