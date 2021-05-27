<?hh

final class :foo:bar {
  attribute
    /**
     * docs for name
     */
    string name;
}

function main(): void {
  <foo:bar name=""/>;
  //        ^ hover-at-caret
}
