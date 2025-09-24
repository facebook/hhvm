<?hh

// OK: named non-optional param can come after positional optional param
function good1(int $x1 = 1, named int $x2): void {}

// OK: positional non-optional param can come after named optional param
function good3(named int $x1 = 1, int $x2): void {}

// Error on $x3
function bad1(int $x1 = 1, named int $x2, int $x3): void {}

// Error on $x3
function bad2(named int $x1 = 1, int $x2 = 1, int $x3): void {}
