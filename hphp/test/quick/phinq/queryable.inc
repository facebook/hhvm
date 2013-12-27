<?hh

class Queryable {

  public function executeQuery($ast, $args) {
    return Vector{$ast};
  }
}

