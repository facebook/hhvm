<?hh
function take_mixed(mixed $_): void {}

async function gen_int(): Awaitable<int> {
  return 1;
}

function main(): void {
  if (1 < 2) {
    // Should be a quickfix, as configured in quickfixes_to_refactors_config.ml
    take_mixed(async () ==> 1 < 2 ? await gen_int() : 0);
                                    // ^ at-caret
    // important: a refactoring here would require
    // selecting the entire expression to extract,
    // but a quickfix only requires the cursor
    // to be in a sub-span of the error span
  }
}
