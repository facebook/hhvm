//// file1.php
<?hh // partial

class BarImpl {
}

newtype Bar = BarImpl;

//// file2.php
<?hh // partial

<<__Memoize>>
function some_function(Bar $i): void {}
