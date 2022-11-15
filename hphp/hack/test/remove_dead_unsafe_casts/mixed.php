<?hh

function some_redundant_some_not(string $str): void {
  HH\FIXME\UNSAFE_CAST<arraykey, string>($str); // redundant
  HH\FIXME\UNSAFE_CAST<string, int>($str); // not redundant
}
