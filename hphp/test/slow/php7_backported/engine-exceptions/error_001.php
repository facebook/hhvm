<?hh

function alpha() :mixed{
  throw new Error('Foo');
}

function beta() :mixed{
  alpha();
}

function gamma() :mixed{
  beta();
}
<<__EntryPoint>>
function main() :mixed{
  gamma();
}
