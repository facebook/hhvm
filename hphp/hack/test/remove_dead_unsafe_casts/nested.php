<?hh

function both_redundant1(string $str): void {
  HH\FIXME\UNSAFE_CAST<mixed, arraykey>(HH\FIXME\UNSAFE_CAST<arraykey, string>($str));
}

function both_redundant2(string $str): void {
  HH\FIXME\UNSAFE_CAST<mixed, string>(HH\FIXME\UNSAFE_CAST<arraykey, string>($str));
}

function outer_redundant(arraykey $str): void {
  HH\FIXME\UNSAFE_CAST<mixed, arraykey>(HH\FIXME\UNSAFE_CAST<arraykey, string>($str));
}

function inner_redundant(string $str): void {
  HH\FIXME\UNSAFE_CAST<string, int>(HH\FIXME\UNSAFE_CAST<mixed, string>($str));
}
