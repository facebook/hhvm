<?hh

class C {
  public function __construct(
    <<Policied("PUBLIC")>>
    public int $public,
    <<Policied("PRIVATE")>>
    public int $private,
    public int $unrelated,
  ) {}
}

<<InferFlows>>
function intermediate(C $obj, int $data): void {
  $obj->unrelated = $obj->private;
  $obj->public = $data;
  $obj->unrelated = $obj->private;
}

<<InferFlows>>
function writeToPublic(C $obj, int $data): void {
  $obj->unrelated = $obj->private;
  intermediate($obj, $data);
  $obj->unrelated = $obj->private;
}

<<InferFlows>>
function getPrivate(C $obj): int {
  $obj->unrelated = $obj->private;
  $tmp = $obj->private;
  $obj->unrelated = $obj->private;
  return $tmp;
}

function test(C $obj): void {
  $obj->unrelated = $obj->private;
  $private = getPrivate($obj);
  $obj->unrelated = $obj->private;
  writeToPublic($obj, $private);
  $obj->unrelated = $obj->private;
}
