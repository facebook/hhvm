<?hh

function get_section_map(string $section_map_file): array<string, string> {
  $handle = fopen($section_map_file, "r");
  $section_map = array();
  while (($pair = fgetcsv($handle)) !== false) {
    // section number is the key, title is the value
    // remove spaces from title, replace inner spaces with empty string
    $section_map[$pair[0]] = str_replace(" ", "", trim($pair[1]));
  }
  return $section_map;
}

function insert_cross_references(string $md_file,
                                 array<string, string> $section_map,
                                 string $ref_link_text = NULL,
                                 bool $update_only = false): void {
  $contents = file($md_file);
  $update = "";
  // matches
  //   §16.2
  //   [§§]
  //   §A.1.2
  $pattern = "/([\[])?§([A-Z0-9\.]+)([\]])?/";
  $matches = array();
  foreach ($contents as $line) {
    // Get all the matches on this line of text
    if (preg_match_all($pattern, $line, $matches) > 0) {
      $ref_texts = $matches[2]; // The matches for the characters after §
      foreach ($ref_texts as $ref_text) {
        // Get rid of any trailing . in case the cross reference is the
        // end of a sentence and the match occurred.
        $ref_text = rtrim($ref_text, ".");
        // For each cross reference match we found, replace with
        // markdown link in our map and create a text link to the title
        $text = $ref_link_text === null ? $ref_text : $ref_link_text;
        $replace = "";
        if (!$update_only || ($update_only && empty($matches[0]))) {
          $replace .= "[";
        }
        $replace .= $text;
        if (!$update_only || ($update_only && empty($matches[0]))) {
          $replace .= "]";
        }
        if (!$update_only){
          $replace .= $section_map[$ref_text];
        }
        $line = str_replace("§".$ref_text, $replace, $line);
      }
    }
    $update .= $line;
  }
  file_put_contents($md_file, $update);
}


function map_word_sections_to_markdown_sections(string $section_map_file,
                                                string $toc_file) {
  $toc_contents = file_get_contents($toc_file);
  $toc_start = strpos($toc_contents, "**Table of Contents**");
  $toc_end = strlen($toc_contents);
  $md_toc = substr($toc_contents, $toc_start, $toc_end);

  // e.g., (01-introduction.md#introduction)
  $md_section_pattern = "/\([a-z0-9\-_]+\.md#[a-z0-9\-_]+\)/";

  $matches = array();
  preg_match_all($md_section_pattern, $md_toc, $matches);
  if (empty($matches[0])) {
    die("No table of contents found" . PHP_EOL);
  }
  // $matches[0] will be an array of the matches we found
  // Start at 1 because $refs[0] will actually be the ToC md file that
  // we don't care about linking to.
  $refs = array_slice($matches[0], 1);
  $sm_contents = file($section_map_file, FILE_IGNORE_NEW_LINES);
  $num_elements = count($sm_contents);
  for ($i = 0; $i < $num_elements; $i++) {
    if (!strpos($sm_contents[$i], $refs[$i])) {
      $sm_contents[$i] .= "," . $refs[$i];
    }
  }
  file_put_contents($section_map_file, implode(PHP_EOL, $sm_contents));
}

function usage(): string {
  return "Usage" . PHP_EOL . PHP_EOL
        . "Make sure you run split.php and toc.php before this!!" . PHP_EOL
        . "php xreference.php [options]" . PHP_EOL
        . "-i <table of contents md file>" . PHP_EOL
        . "-m <input markdown file or directory of files>" . PHP_EOL
        . "-s <section map file>" . PHP_EOL
        . "[-t <text used for clickable reference link>" . PHP_EOL
        . "[-u <update reference link text only; don't change actual link>"
        . PHP_EOL . PHP_EOL;
}

function main(array<string> $argv): void {
  $opts = getopt("hi:m:s:t:u");
  if (array_key_exists("h", $opts)) {
    die(usage());
  }
  if (!array_key_exists("m", $opts) &&
      !array_key_exists("s", $opts) &&
      !array_key_exists("i", $opts)) {
    die("Specify, ToC markdown file, input markdown file and input section map"
        . PHP_EOL . PHP_EOL . usage());
  }
  $toc_file = $opts["i"];
  $md_file = $opts["m"];

  $md_files = array();

  // If we are given a directory of md files to cross reference
  if (is_dir($md_file)) {
    $md_files = scandir($md_file);
    // Keep only .md files
    $md_files = array_filter($md_files, function($val) {
                                          if (strpos($val, ".md") !== false) {
                                            return $val;
                                          }
                                        });
    // scandir only returns a list of the files with no path info
    // so add the path info in $md_file via a callback to array_map
    $md_files = array_map(function($val) use ($md_file) {
                            return $md_file . $val;
                          },
                          $md_files);

  } else {
    $md_files[0] = $md_file;
  }
  $section_map_file = $opts["s"];
  $ref_link_text = null;
  if (array_key_exists("t", $opts)) {
    $ref_link_text = $opts["t"];
  }
  $update_only = false;
  if (array_key_exists("u", $opts)) {
    $update_only = true;
  }
  map_word_sections_to_markdown_sections($section_map_file, $toc_file);
  $section_map = get_section_map($section_map_file);
  foreach($md_files as $file) {
    insert_cross_references($file, $section_map, $ref_link_text,
                            $update_only);
  }
}

main($argv);
