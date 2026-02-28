<?hh

/* @lint-ignore */
class    HasFixme          {
  public function why() {
    // Here is a motivating example for the fixmes-suppress-formatting feature.
    // We do not want to format this node because it would move the suppression
    // comment away from the error, causing it to fail to suppress the error:

    /* @lint-ignore */
    f($some_type_error_we_would_like_to_suppress, $xxxxxxxxxx, $yyyyyyyyyy, $zzzzzzzzzz);
  }

  public function cases() {
    // The FIXME on the class definition only suppresses formatting for the
    // first line (containing the tokens "class" "HasFixme" "{"). Statements
    // inside the class, like this one, are still formatted normally.
    $this_is
           + $formatted;

    /* @lint-ignore */
    func($not_formatted,
         $formatted,
         $formatted);

    /* @lint-ignore */
    func($not_formatted,
         $formatted);

    /* @lint-ignore */ func($not_formatted,
                              $not_formatted,
                              $formatted,
                              $formatted);

        /* @lint-ignore */// not formatted
      /* @lint-ignore */  // not formatted
    /* @lint-ignore */ func($not_formatted,
                              $not_formatted,
                              $formatted,
                              $formatted);

    /* @lint-ignore */
    f($some_type_error_we_would_like_to_suppress, $xxxxxxxxxx, $yyyyyyyyyy, $zzzzzzzzzz)
      + $this_part_of_the_expression
        + $is_still_formatted;

    /* @lint-ignore */
    expect(PHPism_FIXME::varrayFuzzyEqualsDarray($config['foo'], vec[
        'apples',
        'oranges',
        'kiwis',
        'pears',
      ]))->toBeTrue();
  }
}
