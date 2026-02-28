<?hh

class APure {
  function __wakeup()[] :mixed{
    echo __CLASS__." wakes up safely.\n";
  }
  function __sleep()[] :mixed{
    echo __CLASS__." sleeps safely.\n";
    return vec[];
  }
}

class ANonPure {
  function __wakeup() :mixed{
    echo __CLASS__." wakes up safely.\n";
  }
  function __sleep() :mixed{
    echo __CLASS__." sleeps safely.\n";
    return vec[];
  }
}

<<__EntryPoint>>
function main() :mixed{
  $pure = new APure();
  $non_pure = new ANonPure();
  serialize($pure) |> unserialize($$);
  serialize($non_pure) |> unserialize($$);
  serialize_pure($pure) |> unserialize_pure($$);
  serialize_pure($non_pure) |> unserialize_pure($$);
}
