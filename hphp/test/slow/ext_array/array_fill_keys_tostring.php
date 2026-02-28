<?hh

class StringableObj { function __toString()[] :mixed{ return 'Hello'; } }

<<__EntryPoint>>
function main() :mixed{
  array_fill_keys(vec[new StringableObj()], 'value');
}
