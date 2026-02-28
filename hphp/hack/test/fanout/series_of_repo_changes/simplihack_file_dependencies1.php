//// a.php
<?hh

function subprompt(): void {
}

//// b.php
<?hh

function prompt(): void {
  subprompt();
}

//// c.php
<?hh
<<file:__EnableUnstableFeatures('simpli_hack')>>

<<__SimpliHack(prompt())>>
class C {}

//////////////

//// d.php
<?hh

function subsubprompt(): void {
}

//// a.php
<?hh

function subprompt(): void {
  subsubprompt();
}

//////////////

//// d.php
<?hh

function subsubprompt(): void {
  // This is a comment
}
