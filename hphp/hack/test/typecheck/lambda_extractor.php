<?hh // strict

final class LamdbaExtractor {
  private static function run(string $program, int $line): void {
    $json = HH\ffp_parse_string($program);
    $funs = HH\ExperimentalParserUtils\find_all_functions($json);
    $function = $funs[$line];
    $bounds = HH\ExperimentalParserUtils\body_bounds($function);
    list($begin, $end) = $bounds;
  }
}
