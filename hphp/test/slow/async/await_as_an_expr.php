<?hh

async function f() { return 1; }

async function g() { var_dump(await f()); }

\HH\Asio\join(g());

