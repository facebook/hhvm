<?hh // strict

function takes_float(float $f): void { }

function test_ok(): void {
  takes_float( 10.0 ** 2.0 );
  takes_float( 10 ** 2.0 );
  takes_float( 10.0 ** 2 );
}

function test_error(): void {
  takes_float( 10 ** 2 );
}
