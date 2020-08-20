<?hh

function takes_one_arg(int $_): void {}
function takes_string(string $_): void {}

function takes_foo(): void {
  takes_string(1); // error 4110
  takes_one_arg(123, 1); // error 4105
}
