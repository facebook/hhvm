<?hh // strict

class G<+TCov,-TCont,TInv> { }

function test(G<int,string,bool> $g): G<int,string,bool> {
  return $g;
}
