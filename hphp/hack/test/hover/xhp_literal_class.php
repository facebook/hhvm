<?hh

/**
 * docs for :foo:bar
 */
final class :foo:bar {
  attribute string name;
}

function main(): void {
  <foo:bar name=""/>;
  //   ^ hover-at-caret
}
