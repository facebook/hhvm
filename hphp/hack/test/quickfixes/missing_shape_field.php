<?hh

function takes_s(shape('x' => float, 'y' => string) $_): void {}

function foo(): void {
  takes_s(shape('x' => 1.0));
}
