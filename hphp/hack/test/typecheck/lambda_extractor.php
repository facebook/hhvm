<?hh // strict

final class LamdbaExtractor {
  private static function run(string $file, int $line): void {
    $json = HH\ffp_parse_file($file);
    $function = HH\ExperimentalParserUtils\find_single_function($json, $line);
    invariant($function !== null, "Failed to find function");
    $bounds = HH\ExperimentalParserUtils\body_bounds($function);
    list($begin, $end) = $bounds;

    $funs = HH\ExperimentalParserUtils\find_all_functions($json);
    $function = $funs[$line];
    $bounds = HH\ExperimentalParserUtils\body_bounds($function);
    list($begin, $end) = $bounds;
  }
}
