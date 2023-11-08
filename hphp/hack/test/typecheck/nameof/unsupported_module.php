//// file1.php
<?hh

module a.b;

//// file2.php
<?hh

<<file: __EnableUnstableFeatures('nameof_class')>>
function f(): void {
  nameof a.b;
}
