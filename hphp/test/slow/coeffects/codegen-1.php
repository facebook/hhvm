<?hh

class C {
  const ctx C = [codegen_unsafe];
}

function defaults() {}
function f(mixed $x)[$x::C]{ defaults(); }

<<__EntryPoint>>
function main()[] {
  f(new C);
}
