<?hh

async function g(): Awaitable<int> {
  return 1;
}

async function f(): Awaitable<int> {
  $q = async {
    $a = await g();
    return $a;
  };
  return await $q;
}

function h(): Awaitable<int> {
  return async {
    return 1;
  };
}

$a = async {
  return 1;
};
