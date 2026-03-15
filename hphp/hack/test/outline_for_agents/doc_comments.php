<?hh

/**
 * A class with doc comments to verify line range accuracy.
 */
class DocCommentExample {
  /** The name */
  public string $name = "default";

  /**
   * Does something important.
   */
  public function doThing(): void {
    // body
  }
}

// This is a regular comment, not a doc comment
function standalone(): void {
  echo "hello";
}
