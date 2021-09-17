<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

class :my-tag {
  attribute mixed x;
  public function __construct(
    dict<string, mixed> $attrs,
    vec<mixed> $children,
    string $file,
    int $line,
  ) {}
}

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = Code`<my-tag x="y">foo <my-tag /></my-tag>`;

  print_et($et);
}
