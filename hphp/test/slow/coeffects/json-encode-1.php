<?hh

class APure implements JsonSerializable {
  public function jsonSerialize()[] {
    return 'a_pure';
  }
}

class ANonPure implements JsonSerializable {
  public function jsonSerialize() {
    return 'a_non_pure';
  }
}

<<__EntryPoint>>
function main() {
  $pure = new APure();
  $non_pure = new ANonPure();
  json_encode($pure);
  json_encode($non_pure);
  $tmp = null;
  json_encode_pure($pure, inout $tmp);
  json_encode_pure($non_pure, inout $tmp);
}
