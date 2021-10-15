<?hh

function expectDynamic(dynamic $d): void {}

function returnDynamic(): dynamic {
  expectDynamic(1); // error

  return 1; // error
}
