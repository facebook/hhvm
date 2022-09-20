<?hh

function expectDynamic(dynamic $d): void {}

function returnDynamic(): dynamic {
  expectDynamic(1);

  return 1;
}
