---
title: IntlDatePatternGenerator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Generates localized date and/or time format pattern strings suitable for use
in IntlDateFormatter




Transforms unordered skeleton formats like "MMddyyyy" to use the correct
ordering and separators for the locale (for example, one locale might use
"dd-MM-yyyy" when another uses "yyyy/MM/dd").




See Unicode UTS #35 appendix F (Date Format Patterns) for valid input format
patterns:
http://unicode.org/reports/tr35/tr35-6.html#Date_Format_Patterns




Example usage:
$locale = 'en_US';
$generator = IntlDatePatternGenerator::createInstance($locale);
$pattern = $generator->getBestPattern('MMddyyyy');
$formatter = IntlDateFormatter::create($locale, null, null);
$formatter->setPattern($pattern);
$date = $formatter->format(new DateTime());




Constants:




Pattern fields:
IntlDatePatternGenerator::ERA_PATTERN_FIELD
IntlDatePatternGenerator::YEAR_PATTERN_FIELD
IntlDatePatternGenerator::QUARTER_PATTERN_FIELD
IntlDatePatternGenerator::MONTH_PATTERN_FIELD
IntlDatePatternGenerator::WEEK_OF_YEAR_PATTERN_FIELD
IntlDatePatternGenerator::WEEK_OF_MONTH_PATTERN_FIELD
IntlDatePatternGenerator::WEEKDAY_PATTERN_FIELD
IntlDatePatternGenerator::DAY_OF_YEAR_PATTERN_FIELD
IntlDatePatternGenerator::DAY_OF_WEEK_IN_MONTH_PATTERN_FIELD
IntlDatePatternGenerator::DAY_PATTERN_FIELD
IntlDatePatternGenerator::DAYPERIOD_PATTERN_FIELD
IntlDatePatternGenerator::HOUR_PATTERN_FIELD
IntlDatePatternGenerator::MINUTE_PATTERN_FIELD
IntlDatePatternGenerator::SECOND_PATTERN_FIELD
IntlDatePatternGenerator::FRACTIONAL_SECOND_PATTERN_FIELD
IntlDatePatternGenerator::ZONE_PATTERN_FIELD




Pattern conflict status:
IntlDatePatternGenerator::PATTERN_NO_CONFLICT
IntlDatePatternGenerator::PATTERN_BASE_CONFLICT
IntlDatePatternGenerator::PATTERN_CONFLICT




## Interface Synopsis




``` Hack
class IntlDatePatternGenerator {...}
```




### Public Methods




+ [` ::createEmptyInstance(): IntlDatePatternGenerator `](/apis/Classes/IntlDatePatternGenerator/createEmptyInstance/)\
  Creates an empty generator, to be constructed with addPattern(...) etc

+ [` ::createInstance(string $locale): IntlDatePatternGenerator `](/apis/Classes/IntlDatePatternGenerator/createInstance/)\
  Creates a flexible generator according to the data for a given locale

+ [` ->addPattern(string $pattern, bool $override): int `](/apis/Classes/IntlDatePatternGenerator/addPattern/)\
  Adds a pattern to the generator

+ [` ->getAppendItemFormat(int $field): string `](/apis/Classes/IntlDatePatternGenerator/getAppendItemFormat/)

+ [` ->getAppendItemName(int $field): string `](/apis/Classes/IntlDatePatternGenerator/getAppendItemName/)

+ [` ->getBaseSkeleton(string $pattern): string `](/apis/Classes/IntlDatePatternGenerator/getBaseSkeleton/)\
  Utility to return a unique base skeleton from a given pattern

+ [` ->getBaseSkeletons(): IntlIterator `](/apis/Classes/IntlDatePatternGenerator/getBaseSkeletons/)

+ [` ->getBestPattern(string $skeleton): string `](/apis/Classes/IntlDatePatternGenerator/getBestPattern/)\
  Returns the best pattern matching the input skeleton

+ [` ->getDateTimeFormat(): string `](/apis/Classes/IntlDatePatternGenerator/getDateTimeFormat/)

+ [` ->getDecimal(): string `](/apis/Classes/IntlDatePatternGenerator/getDecimal/)\
  The decimal value is used in formatting fractions of seconds

+ [` ->getErrorCode(): int `](/apis/Classes/IntlDatePatternGenerator/getErrorCode/)\
  Get last error code on the object

+ [` ->getErrorMessage(): string `](/apis/Classes/IntlDatePatternGenerator/getErrorMessage/)\
  Get last error message on the object

+ [` ->getPatternForSkeleton(string $skeleton): string `](/apis/Classes/IntlDatePatternGenerator/getPatternForSkeleton/)\
  Get the pattern corresponding to a given skeleton

+ [` ->getSkeleton(string $pattern): string `](/apis/Classes/IntlDatePatternGenerator/getSkeleton/)\
  Utility to return a unique skeleton from a given pattern

+ [` ->getSkeletons(): IntlIterator `](/apis/Classes/IntlDatePatternGenerator/getSkeletons/)

+ [` ->replaceFieldTypes(string $pattern, string $skeleton): string `](/apis/Classes/IntlDatePatternGenerator/replaceFieldTypes/)\
  Adjusts the field types (width and subtype) of a pattern to match what is
  in a skeleton

+ [` ->setAppendItemFormat(int $field, string $value): void `](/apis/Classes/IntlDatePatternGenerator/setAppendItemFormat/)\
  An append item format is a pattern used to append a field if there is no
  good match

+ [` ->setAppendItemName(int $field, string $name): void `](/apis/Classes/IntlDatePatternGenerator/setAppendItemName/)\
  Sets the name of a field, eg "era" in English for ERA

+ [` ->setDateTimeFormat(string $dateTimeFormat): void `](/apis/Classes/IntlDatePatternGenerator/setDateTimeFormat/)\
  The date time format is a message format pattern used to compose date and
  time patterns

+ [` ->setDecimal(string $decimal): void `](/apis/Classes/IntlDatePatternGenerator/setDecimal/)\
  The decimal value is used in formatting fractions of seconds

<!-- HHAPIDOC -->
