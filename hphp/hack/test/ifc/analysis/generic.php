<?hh // strict

class G<+TCov,-TCont,TInv> { }

function testVariance(G<int,string,bool> $g): G<int,string,bool> {
  return $g;
}

function id<T>(T $t): T {
  return $t;
}
