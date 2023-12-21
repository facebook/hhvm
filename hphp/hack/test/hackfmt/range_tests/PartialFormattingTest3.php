<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/**
 * Static functions supporting nothing
 */
abstract final class PartialFormattingTest {

  // Comment1
  // Comment2
  public static async function genMaybeValidateID(
    ?int $id = null,
    bool $validate = false,
  ): Awaitable<?int> {

    $nonsensical_maplike_array = dict[
      "A Very Long Key Value For This Map" => "Here's a value of an appropriate length"
    ];

    if (false) {
      /**
       * TODO: Do something
       * MultilineComment1
       *
       * MultilineComment1-2
       */
      $errors = vec[];
      $create_exception = null;
    }

    // Comment 3
    // Comment 4
    // Comment 5
    return $id;
  }
}
