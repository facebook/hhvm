<?hh

function test_otf(mixed ...$_): void {
}

function otf_1(): void {
  test_otf(
    '1234567890',
    '1234567890',
    '1234567890','1234567890', '1234567890','1234567890'); // 11: Format on ";"
}




function otf(): void {}      // 13: Format on "}"
