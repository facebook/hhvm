<?hh

class C2 {
  const CONST1 = C1::CONST1;
}

class C3 {
  const CONST1 = vec[C2::CONST1];
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C3::CONST1);
}
