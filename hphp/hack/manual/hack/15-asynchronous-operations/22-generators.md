# Generators

[Generators](http://php.net/manual/en/language.generators.overview.php) provide a more compact way to write an
[iterator](http://php.net/manual/en/language.oop5.iterations.php). Generators work by passing control back and forth between the
generator and the calling code. Instead of returning once or requiring something that could be memory-intensive like an array, generators
yield values to the calling code as many times as necessary in order to provide the values being iterated over.

Generators can be `async` functions; an async generator behaves similarly to a normal generator except that each yielded value is an
`Awaitable` that is `await`ed upon.

## Async Iterators

To yield values or key/value pairs from async generators, we return [HH\AsyncIterator](/apis/Interfaces/HH/AsyncIterator/) or
[HH\AsyncKeyedIterator](/apis/Interfaces/HH/AsyncKeyedIterator/), respectively.

Here is an example of using the [async utility function](/hack/asynchronous-operations/utility-functions)
[`usleep`](/apis/Functions/HH.Asio/usleep/) to imitate a second-by-second countdown clock. Note that in the
`happy_new_year` `foreach` loop we have the syntax `await as`. This is shorthand for calling `await $ait->next()`.

```hack
const int SECOND = 1000000; // microseconds

async function countdown(int $from): AsyncIterator<int> {
  for ($i = $from; $i >= 0; --$i) {
    await \HH\Asio\usleep(SECOND);
    // Every second, a value will be yielded back to the caller, happy_new_year()
    yield $i;
  }
}

async function happy_new_year(int $start): Awaitable<void> {
  // Get the AsyncIterator that enables the countdown
  $ait = countdown($start);
  foreach ($ait await as $time) {
    // we are awaiting the returned awaitable, so this will be an int
    if ($time > 0) {
      echo $time."\n";
    } else {
      echo "HAPPY NEW YEAR!!!\n";
    }
  }
}

<<__EntryPoint>>
function run(): void {
  \HH\Asio\join(happy_new_year(5)); // 5 second countdown
}
```

We must use `await as`; otherwise we'll not get the iterated value.

Although `await as` is just like calling `await $gen->next()`, we should always use `await as`. Calling the
[`AsyncGenerator`](/apis/Classes/HH/AsyncGenerator/) methods directly is rarely needed. Also note that on async iterators,
`await as` or a call to [`next`](/apis/Classes/HH/AsyncGenerator/next/) actually returns a value (instead of `void` like in a normal iterator).

## Sending and Raising

**Calling these methods directly should be rarely needed; `await as` should be the most common way to access values returned by an iterator.**

We can send a value to a generator using [`send`](/apis/Classes/HH/AsyncGenerator/send/) and raise an exception upon a
generator using [`raise`](/apis/Classes/HH/AsyncGenerator/raise/).

If we are doing either of these two things, our generator must return `AsyncGenerator`. An `AsyncGenenator` has three type
parameters: the key, the value. And the type being passed to [`send`](/apis/Classes/HH/AsyncGenerator/send/).

```hack
const int HALF_SECOND = 500000; // microseconds

async function get_name_string(int $id): Awaitable<string> {
  // simulate fetch to database where we would actually use $id
  await \HH\Asio\usleep(HALF_SECOND);
  return \str_shuffle("ABCDEFG");
}

async function generate(): AsyncGenerator<int, string, int> {
  $id = yield 0 => ''; // initialize $id
  // $id is a ?int; you can pass null to send()
  while ($id is nonnull) {
    $name = await get_name_string($id);
    $id = yield $id => $name; // key/string pair
  }
}

async function associate_ids_to_names(vec<int> $ids): Awaitable<void> {
  $async_generator = generate();
  // You have to call next() before you send. So this is the priming step and
  // you will get the initialization result from generate()
  $result = await $async_generator->next();
  \var_dump($result);

  foreach ($ids as $id) {
    // $result will be an array of ?int and string
    $result = await $async_generator->send($id);
    \var_dump($result);
  }
}

<<__EntryPoint>>
function run(): void {
  $ids = vec[1, 2, 3, 4];
  \HH\Asio\join(associate_ids_to_names($ids));
}
```

Here is how to raise an exception to an async generator.

```hack
const int HALF_SECOND = 500000; // microseconds

async function get_name_string(int $id): Awaitable<string> {
  // simulate fetch to database where we would actually use $id
  await \HH\Asio\usleep(HALF_SECOND);
  return \str_shuffle("ABCDEFG");
}

async function generate(): AsyncGenerator<int, string, int> {
  $id = yield 0 => ''; // initialize $id
  // $id is a ?int; you can pass null to send()
  while ($id is nonnull) {
    $name = "";
    try {
      $name = await get_name_string($id);
      $id = yield $id => $name; // key/string pair
    } catch (\Exception $ex) {
      \var_dump($ex->getMessage());
      $id = yield 0 => '';
    }
  }
}

async function associate_ids_to_names(vec<int> $ids): Awaitable<void> {
  $async_generator = generate();
  // You have to call next() before you send. So this is the priming step and
  // you will get the initialization result from generate()
  $result = await $async_generator->next();
  \var_dump($result);

  foreach ($ids as $id) {
    if ($id === 3) {
      $result = await $async_generator->raise(
        new \Exception("Id of 3 is bad!"),
      );
    } else {
      $result = await $async_generator->send($id);
    }
    \var_dump($result);
  }
}

<<__EntryPoint>>
function run(): void {
  $ids = vec[1, 2, 3, 4];
  \HH\Asio\join(associate_ids_to_names($ids));
}
```
