// Continuation type is not supported  <?hh // strict

namespace NS_yield;

// define a simple generator that returns a series of consecutive values

//function series(int $start, int $end): Generator<int, int, void> {
function series(int $start, int $end): Continuation<int> {
  for ($i = $start; $i <= $end; ++$i) {
    yield $i;
  }
}

// define a simple generator that returns key/value pairs

function series2(int $start, int $end, string $keyPrefix = ""): Generator<string, int, void> {
  for ($i = $start; $i <= $end; ++$i) {
    yield $keyPrefix . $i => $i;	// specify a key/value pair
  }
}

// define a generator that returns sucessive lines from a file of text

//function getTextFileLines(string $filename): Generator<int, string, void> {
function getTextFileLines(string $filename): Continuation<string> {
  $infile = fopen($filename, 'r');
  if ($infile == false) {
    // handle file-open failure
  }

  try {
    while ($textLine = fgets($infile)) { // while not EOF
//    echo "len before rtrim: " . + strlen($textLine) . "\n";
      $textLine = rtrim($textLine, "\r\n");	// strip off line terminator
//    echo "len after rtrim:  " . + strlen($textLine) . "\n";
      yield $textLine;
    }
  }
  finally {
    fclose($infile);
  }
}

function main(): void {
  foreach (series(1, 5) as $key => $val) {
    echo "key: $key, value: $val\n";
  }

  echo "===========================\n";

  foreach (series2(1, 5, "X") as $key => $val) {
    echo "key: $key, value: $val\n";
  }

  echo "===========================\n";

  foreach (getTextFileLines("Testfile.txt") as $line) {
    echo ">$line<\n";
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();
