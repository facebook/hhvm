<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function setDefault<Tk as arraykey, Tv>(
    Map<Tk, Tv> $map,
    Tk $key,
    Tv $default,
  ): Map<Tk, Tv> {
  return Map {};
}

type GraphNodeShape<Tkey as arraykey> = shape(
  'id' => Tkey,
  ...
);

final class Graph<Tkey as arraykey, Tnode as GraphNodeShape<Tkey>> {

  private function __construct(
    private Map<Tkey, (Tnode, Set<Tkey>)> $nodeMap,
  ) {
  }

   public static function fromEdges(Iterable<(Tnode, Tnode)> $edges): this {
    $node_map = Map {};
    foreach ($edges as $edge) {
      list($s, $t) = $edge;
      $x = $s['id'];
      $y = $t['id'];
      setDefault($node_map, $x, tuple($s, Set {}));
      $node_map[$x][1]->add($y);
    }

    return new self($node_map);
  }
}
