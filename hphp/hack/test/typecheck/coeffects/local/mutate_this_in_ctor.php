<?hh

final class GraphQLTypeSet {
  public varray<string> $typeSets;
  public function __construct(string $test)[] {
    $this->typeSets = varray[];
    $this->typeSets[] = $test;
  }
}
