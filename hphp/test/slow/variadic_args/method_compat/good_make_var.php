<?hh

interface DB {
  public function query($query);
}

class MySQL implements DB {
  public function query($query, ...$params) { }
}
