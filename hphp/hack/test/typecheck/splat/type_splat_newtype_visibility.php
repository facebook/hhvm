//// defining.php
<?hh

newtype TupleArgs = (int, string);

function inside_defining_file((...TupleArgs) $_): void {}

//// using.php
<?hh

function outside_defining_file((...TupleArgs) $_): void {}
