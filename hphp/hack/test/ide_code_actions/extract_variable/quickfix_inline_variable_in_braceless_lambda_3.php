<?hh

function take_mixed(mixed $_): void {}

async function gen_int(): Awaitable<int> {
  return 1;
}

function main(): void {
  if (1 < 2) {
    // Should be a quickfix, as configured in quickfixes_to_refactors_config.ml
    take_mixed(async () ==> 1 < 2 ? /*range-start*/await gen_int()/*range-end*/ : 0);
  }
}
