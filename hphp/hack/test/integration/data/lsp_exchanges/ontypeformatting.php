<?hh //partial

function test_otf(mixed ...$_): void {
}

function otf_1() {
  test_otf(
    '1234567890',
    '1234567890',
    '1234567890','1234567890', '1234567890','1234567890'); // 11: Format on ";"
}


{}     // 12: Format on bracket-matching "{}"

function otf(){}      // 13: Format on "}"
