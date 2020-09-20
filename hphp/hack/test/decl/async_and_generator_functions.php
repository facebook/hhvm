<?hh

function sync_function(): void {}

async function async_function(): Awaitable<string> {
    await HH\Asio\usleep(500000);
    return "test";
}

function generator_function(): Generator<string, int, void> {
    yield "test" => 5;
}

function generator_function_implicit_key(): Generator<int, string, void> {
    yield "test";
}

async function async_generator(): AsyncGenerator<int, string, void> {
    $value = await async_function();
    yield 0 => $value;
}
