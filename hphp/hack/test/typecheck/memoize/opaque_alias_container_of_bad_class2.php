//// file1.php

<?hh

class BarImpl {
}

newtype Bar = array<BarImpl>;

//// file2.php

<?hh

<<__Memoize>>
function some_function(Bar $i): void {}
