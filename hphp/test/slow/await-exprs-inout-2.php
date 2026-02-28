<?hh

<<__EntryPoint>>
async function main() {
  $a1 = 10;
  $b1 = 100;
  gs(io(inout $a1, inout $b1, await g(1)));
}
