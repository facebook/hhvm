<?hh

async function gen_usleep(int $usecs): Awaitable<void> {
    await SleepWaitHandle::create($usecs);
}

function get(): void { var_dump(HH\get_product_attribution_id()); }

function indirection(): void { get(); }

function set_and_get($x): void {
  HH\set_product_attribution_id($x);
  get();
}

function make_get_closure_with_message(string $message): (function(): void) {
    return HH\embed_product_attribution_id_in_closure(() ==> {
        echo $message."\n";
        get();
    });
}

function make_async_get_closure_with_message(string $message): (function(): Awaitable<void>) {
    return HH\embed_product_attribution_id_in_async_closure(async () ==> {
        // Force a sleep wait
        await gen_usleep(1);
        echo $message."\n";
        get();
    });
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
    // Test sync closures
    $queue = vec[];
    $queue[] = make_get_closure_with_message("Not set");

    HH\set_product_attribution_id(null);
    $queue[] = make_get_closure_with_message("Set null");

    HH\set_product_attribution_id(5);
    $queue[] = make_get_closure_with_message("Set 5");

    HH\set_product_attribution_id_deferred(()[leak_safe] ==> { return 10; });
    $queue[] = make_get_closure_with_message("Set lambda 10");

    HH\set_product_attribution_id(15);
    $queue[] = HH\embed_product_attribution_id_in_closure(() ==> {
        echo "Set to 15 (get via indirection)\n";
        indirection();
    });

    HH\set_product_attribution_id(20);
    $queue[] = HH\embed_product_attribution_id_in_closure(() ==> {
        echo "Nested sets 20-25\n";
        set_and_get(25);
        get();
    });
    HH\set_product_attribution_id(null);
    foreach ($queue as $closure) {
        $closure();
    }

    // Test async closures
    $queue = vec[];
    HH\set_product_attribution_id(null);
    $queue[] = make_async_get_closure_with_message("Set null");

    HH\set_product_attribution_id(5);
    $queue[] = make_async_get_closure_with_message("Set 5");

    HH\set_product_attribution_id_deferred(()[leak_safe] ==> { return 10; });
    $queue[] = make_async_get_closure_with_message("Set lambda 10");

    HH\set_product_attribution_id(15);
    $queue[] = HH\embed_product_attribution_id_in_async_closure(async () ==> {
        await gen_usleep(1);
        echo "Set to 15 (get via indirection)\n";
        indirection();
    });

    HH\set_product_attribution_id(20);
    $queue[] = HH\embed_product_attribution_id_in_async_closure(async () ==> {
        await gen_usleep(1);
        echo "Nested sets 20-25\n";
        set_and_get(25);
        get();
    });
    HH\set_product_attribution_id(null);
    foreach ($queue as $closure) {
        await $closure();
    }
}
