<?hh

function f<Tk as arraykey>(dict<Tk, string> $_): void where Tk = int {}

function g(): string {
  return 'ouch!';
}

function test2(): void {
  f(dict['foo' => g()]);
}
