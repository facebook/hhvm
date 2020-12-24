<?hh

function impure(): void {}

function pure()[]: void {
  impure();
}
