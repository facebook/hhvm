<?hh

async function gen_awaitable_creation_expression_test(): Awaitable<void> {
  await async {
    $bar = await async {
      return get_bar();
    };
    $foo = await gen_value_of_foo_from_bar($bar);
    return $foo;
  } |> gen_set_value_of_with_awaitable_foo($$);

  async {
    return await gen_simple();
  };
}
