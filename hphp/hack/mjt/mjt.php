<?hh

class Blah {

    const type TTypeCaseShape = shape(
    // Cases to run with no parameters
    ?'cases' => ?vec<(mixed, mixed)>,
    // Cases to run with parameters to the XParam::Foo factory
    ?'param_cases' => ?vec<shape(
      'params' => vec<mixed>,
      'cases' => vec<(mixed, mixed)>,
    )>,
   );

  private static function typeCases(
      ): dict<string, (mixed, self::TTypeCaseShape)> {
        return dict[
          'Int' => tuple(
            "a",
            shape(
              'cases' => vec[
                tuple('-1', -1),
                tuple('0', 0),
                tuple('1', 1),
                tuple('a', null),
                tuple('a1', null),
                tuple('1a', null),
                tuple(1, 1),
                tuple(null, null),
              ],
            ),
          )];
      }

  public static function provideTypeCases(): vec<vec<mixed>> {
    $runs = vec[];
    foreach (self::typeCases() as list($func, $case_data)) {
      $all_cases = Shapes::idx($case_data, 'param_cases', vec[]) as vec<_>;
      $all_cases[] = shape(
        'cases' => Shapes::idx($case_data, 'cases', vec[]),
        'params' => vec[],
      );

      foreach ($all_cases as $param_case) {
        $cases = $param_case['cases'] as Traversable<_>;
        foreach ($cases as $case) {
          list($input, $output) = $case;
          $runs[] = vec[
            $func,
            false,
            $input,
            $output,
            $param_case['params'],
          ];
        }
      }
    }
    return $runs;
  }
}
