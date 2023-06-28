<?hh


class Frob {
  const FROBS_PER_BLOB = 24;
}

class Widget {
  private $count = 5 * Frob::FROBS_PER_BLOB;
}


<<__EntryPoint>>
function main_deep_init() :mixed{
var_dump(new Widget);
}
