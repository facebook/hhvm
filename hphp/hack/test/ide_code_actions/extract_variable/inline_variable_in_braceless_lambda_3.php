<?hh

function take_mixed(mixed $_): void {}

async function gen_int(): Awaitable<int> {
  return 1;
}

function main(): void {
  if (1 < 2) {
    take_mixed(async () ==> 1 < 2 ? /*range-start*/await gen_int()/*range-end*/ : 0);
  }
}
