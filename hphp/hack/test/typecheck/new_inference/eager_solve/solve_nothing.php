<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


final class MySet<Tv> {

  private Set<arraykey> $index = Set {};

  public function __construct(
    private (function(Tv): arraykey) $keyFunction,
  ) {}

  private function getKey(Tv $obj): arraykey {
    $func = $this->keyFunction;
    return $func($obj);
  }

  public function put(Tv $obj): void {
    $this->index[] = $this->getKey($obj);
  }

}

function testit(vec<dict<string,int>> $rls_rows):void {
  $pk_bitmap = new MySet(
    $t ==> { return $t[0].$t[1].$t[2]; },
  );
  foreach ($rls_rows as $row) {
    $schema_key = $row['schema_key'];
    $result_id = $row['result_id'];
    $job_id = $row['job_id'];
    $result_type = $row['result_type'];

    $pk_bitmap->put(tuple($schema_key, $result_id, $result_type));
  }
}
