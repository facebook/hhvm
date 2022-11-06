<?hh

function nonreturn_happly_void(Awaitable<void> $x): void {}

function nonreturn_happly_void(Awaitable<noreturn> $x): void {}
