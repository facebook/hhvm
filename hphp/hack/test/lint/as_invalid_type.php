<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum HackAsInvalidTypeStringEnum: string {
  VALUE = 'zero';
}

final class HackAsWithInvalidTypeLintRuleTest {

  private static function bool(): bool {
    return false;
  }

  private static function int(): int {
    return 0;
  }

  private static function string(): string {
    return "0";
  }

  private static function float(): float {
    return 0.0;
  }

  private static function arraykey(): arraykey {
    return "0";
  }

  private static function num(): num {
    return 0;
  }

  private static function mixed(): mixed {
    return false;
  }

  private static function nullable<T>(T $thing): ?T {
    return $thing;
  }

  private static function null(): null {
    return null;
  }

  private static function stringEnum(): HackAsInvalidTypeStringEnum {
    return HackAsInvalidTypeStringEnum::VALUE;
  }

  public static function foo(): void {
    $_ = self::int() as bool; // error, never succeeds
    $_ = self::bool() as bool; // error, always succeeds
    $_ = self::float() as num; // error, always succeeds
    $_ = self::num() as float;
    $_ = self::float() as int; // error, never succeeds
    $_ = self::arraykey() as num;
    $_ = self::string() as arraykey; // error, always succeeds
    $_ = self::string() as int; // error, never succeeds
    $_ = self::mixed() as int;
    $_ = self::int() as mixed; // error, always succeeds
    $_ = self::nullable(self::string()) as arraykey;
    $_ = self::nullable(self::string()) as ?string; // error, always succeeds
    $_ = self::nullable(self::string()) as ?int;
    $_ = self::stringEnum() as string;
    $_ = self::stringEnum() as ?string;
    $_ = self::stringEnum() as int;
    $_ = self::null() as mixed; // error, always succeeds
    $_ = self::null() as null; // error, always succeeds
    $_ = self::null() as ?int; // error, always succeeds
    $_ = self::nullable(self::int()) as null;
    $_ = self::null() as int; // error, always succeeds
    $_ = self::int() as null; // error, never succeeds
  }
}
