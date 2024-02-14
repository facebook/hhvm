<?hh

function take_mixed(mixed $_): void {}

function main(): void {
  take_mixed(() ==> 1 < 2 ? /*range-start*/1 + 2/*range-end*/ : 0);
}
