<?hh

foo((...$x, $y) ==> {});

foo((int ...$x, int $y) ==> {});

foo((..., $y) ==> {});

foo((int ...$x,) ==> {});

foo(function (int ...$x, int $y): void {});

foo(function (...$x, int $y): void {});

foo(function (..., int $y): void {});

foo((inout int ...$x) ==> {});

foo(function (inout int ...$x): void {});
