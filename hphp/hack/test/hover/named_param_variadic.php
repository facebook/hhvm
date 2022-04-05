<?hh

function add_one(string $s, int ...$nums): void {
}

function call_it(): void {
  add_one("stuff", 1, 2);
  //                  ^ hover-at-caret
}
