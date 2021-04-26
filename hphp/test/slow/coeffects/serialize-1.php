<?hh

class APure {
  function __wakeup()[] {
    echo __CLASS__." wakes up safely.\n";
  }
  function __sleep()[] {
    echo __CLASS__." sleeps safely.\n";
    return varray[];
  }
}

class ANonPure {
  function __wakeup() {
    echo __CLASS__." wakes up safely.\n";
  }
  function __sleep() {
    echo __CLASS__." sleeps safely.\n";
    return varray[];
  }
}

<<__EntryPoint>>
function main() {
  $pure = new APure();
  $non_pure = new ANonPure();
  serialize($pure) |> unserialize($$);
  serialize($non_pure) |> unserialize($$);
  serialize_pure($pure) |> unserialize_pure($$);
  serialize_pure($non_pure) |> unserialize_pure($$);
}
