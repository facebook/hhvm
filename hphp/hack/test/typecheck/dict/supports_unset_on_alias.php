//// file1.php
<?hh // strict

newtype aliased_dict as dict<string, int> = dict<string, int>;

//// file2.php
<?hh // strict

function test(aliased_dict $x): void {
  unset($x['a']);
}
