<?hh

function alpha() {
  throw new Error('Foo');
}

function beta() {
  alpha();
}

function gamma() {
  beta();
}
<<__EntryPoint>>
function main() {
  gamma();
}
