<?hh

class C {
  static vec<int> $x = vec[];
  static vec<Foo> $y = vec[];
}

class Foo {}

<<__EntryPoint>>
function main() {
  C::$x[] = 17;
  C::$y[] = new Foo();

  $hg = heapgraph_create();
  $static_props = Vector {};

  heapgraph_foreach_node($hg, $node ==> {
    if (idx($node, 'type') === 'HPHP::StaticPropData') {
      $static_props[] = $node['class'].'::'.$node['prop'];
    }
  });

  $static_props = vec($static_props);
  sort(inout $static_props);
  var_dump($static_props);
}
