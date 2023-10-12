<?hh // strict

async function g(): Awaitable<int> {
  return 1;
}

async function f(): Awaitable<string> {
  $q = async {
    $a = await g();
    return $a;
  };
  return await $q;
}
