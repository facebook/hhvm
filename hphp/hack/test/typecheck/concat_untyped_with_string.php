//// partial.php
<?hh

function any() {
}

//// strict.php
<?hh // strict

function f(): string {
  return 'a' . any();
}
