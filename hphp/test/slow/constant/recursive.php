<?hh

const A = B;
const B = A;

<<__EntryPoint>>
function main() {
A;
B;
echo "OK\n";
}
