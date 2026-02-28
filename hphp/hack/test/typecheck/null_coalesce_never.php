<?hh

type S = shape('a' => vec<int>);

function foo(S $s): void {
  Shapes::at($s, 'a') ?? vec[];
}
