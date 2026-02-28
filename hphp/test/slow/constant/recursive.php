<?hh

const A = B;
const B = A;

<<__EntryPoint>>
function main() :mixed{
A;
B;
echo "OK\n";
}
