<?hh //partial

function foo($x): void {
    $y = $x;
    bar($y);
}

function bar(int $_): void {}
