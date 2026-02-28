<?hh
// This file IS in a "generated" directory, so warnings should be SUPPRESSED

abstract class Generated {
  public static function m1(): void {
    // No warning expected because this file matches /generated/ pattern
    static::abs();
  }
  public static abstract function abs(): void;
}
