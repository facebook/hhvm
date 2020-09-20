<?hh

async function f() { return 1; }

async function g() { var_dump(await f()); }



<<__EntryPoint>>
function main_await_as_an_expr() {
\HH\Asio\join(g());
}
