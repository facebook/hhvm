//// partial.php
<?hh // partial

function any() {
}

//// strict.php
<?hh // strict

function f(): string {
  return 'a' . any();
}
