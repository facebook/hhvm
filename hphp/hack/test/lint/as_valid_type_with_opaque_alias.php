//// my_alias.php
<?hh

newtype MyIntID_of = int;

//// use_it.php
<?hh

function foo(?MyIntID_of $x): void {
  // Might throw, might not. The lint shouldn't fire here, because we
  // only report unconditional throw situations.
  $x as int;
}
