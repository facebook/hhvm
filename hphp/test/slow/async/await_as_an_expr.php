<?hh

async function f() :Awaitable<mixed>{ return 1; }

async function g() :Awaitable<mixed>{ var_dump(await f()); }



<<__EntryPoint>>
function main_await_as_an_expr() :mixed{
\HH\Asio\join(g());
}
