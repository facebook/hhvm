<?hh

class CNonPure {
  public function __debugInfo(): dict<string, string> {
    echo __CLASS__." invokes ".__METHOD__." safely.\n";
    return dict[];
  }
}

class CPure {
  public function __debugInfo()[]: dict<string, string> {
    echo __CLASS__." invokes ".__METHOD__." safely.\n";
    return dict[];
  }
}

<<__EntryPoint>>
function main() :mixed{
  $pure = new CPure();
  $non_pure = new CNonPure();
  print_r($pure);
  print_r($non_pure);
  print_r_pure($pure);
  print_r_pure($non_pure);
}
