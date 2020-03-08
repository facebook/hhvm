<?hh

/**
  Use this to add the language of your choice to Markdown code blocks.
  This does some minimal checking to see if a language has already been
  specified, and assumes the code blocks are of the form:

  ```
  code here
  ```

  with each ``` on a separate line

  Right now, you give the script the language and it does the modification
  for all code blocks
**/

function main(array<string> $argv): void {
  if (!array_key_exists(1, $argv)) {
    die("Specify a language, such as 'Hack'");
  }

  $spec_dir = __DIR__ . "/../spec";

  $di = new RecursiveDirectoryIterator($spec_dir,
                                       RecursiveDirectoryIterator::SKIP_DOTS);
  $it = new RecursiveIteratorIterator($di);

  $code_block_pattern = "/(```)(php)?(\n.*?```\n)/s"; // /s matches newline for .
  // $1 = ```
  // $2 = php  ... match on php in case we used that before Hack was supported
  // $3 = \n.*?```\n
  $fix = "$1" . $argv[1] . "$3";

  foreach($it as $spec_file) {
    if ($spec_file->isFile() && $spec_file->getExtension() === "md" &&
        $spec_file->getFilename() !== "hack-spec-draft.md" &&
        $spec_file->getFilename() !== "00-specification-for-hack.md") {
      $contents = file_get_contents($spec_file);
      $matches = array();
      preg_match_all($code_block_pattern, $contents, $matches);
      $contents = preg_replace($code_block_pattern, $fix, $contents);
      file_put_contents($spec_file, $contents);
    }
  }
}

main($argv);
