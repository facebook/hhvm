<?hh

function rx()[rx] :mixed{}
function pure()[] :mixed{ rx(); }

<<__EntryPoint>>
function main() :mixed{
  pure();
}
