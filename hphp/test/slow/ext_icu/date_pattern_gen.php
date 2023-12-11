<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}

function FAIL() :mixed{
  echo "Failed: \n";
  var_dump(debug_backtrace());
}

function EXPECT_THROWS($function, $catch_block) :mixed{
  try {
    $function();
    FAIL();
  } catch (Exception $e) {
    $catch_block($e);
  }
}

function EXPECT_INVALID_STR($gen, $function) :mixed{
  EXPECT_THROWS($function, function ($e) use ($gen) {
    VS($gen->getErrorCode(), 10);
    VS($gen->getErrorMessage(), 'U_INVALID_CHAR_FOUND');
  });
}

function EXPECT_THROWS_MESSAGE($function, $message) :mixed{
  EXPECT_THROWS($function, $e ==> VS($e->getMessage(), $message));
}

function EXPECT_INVALID_PATTERN_FIELD($value, $function) :mixed{
  EXPECT_THROWS_MESSAGE($function, "Invalid value: $value for pattern field");
}

function EXPECT_NO_LOCALE($function) :mixed{
  EXPECT_THROWS_MESSAGE($function, 'No locale provided');
}

const INVALID_STR = "\xe2\x28\xa1";
const ERA_FIELD = IntlDatePatternGenerator::ERA_PATTERN_FIELD;
const FIELD_COUNT = IntlDatePatternGenerator::PATTERN_FIELD_COUNT;
const MONTH_FIELD = IntlDatePatternGenerator::MONTH_PATTERN_FIELD;
const INVALID_FIELD = -1;

//////////////////////////////////////////////////////////////////////

function test_create_instance_with_no_locale_fails() :mixed{
  EXPECT_NO_LOCALE(function () {
    IntlDatePatternGenerator::createInstance("");
  });
}

function test_create_instance_with_null_locale_fails() :mixed{
  EXPECT_NO_LOCALE(function () {
    IntlDatePatternGenerator::createInstance('');
  });
}

function test_create_instances_with_different_locales() :mixed{
  $gen = IntlDatePatternGenerator::createInstance("en_US");
  VS($gen->getBestPattern('yyyyMMdd'), 'MM/dd/yyyy');

  // Test with invalid english locale (ICU defaults to en_US)
  $gen = IntlDatePatternGenerator::createInstance("en_UK");
  VS($gen->getBestPattern('yyyyMMdd'), 'MM/dd/yyyy');

  $gen = IntlDatePatternGenerator::createInstance("en_GB");
  VS($gen->getBestPattern('yyyyMMdd'), 'dd/MM/yyyy');
}

function test_create_empty_instance() :mixed{
  $gen = IntlDatePatternGenerator::createEmptyInstance();

  // NOTE: This segfaults with ICU 49.1.2 (only with an empty instance)
  // $gen->getBestPattern('yyyyMM');

  VS($gen->getDateTimeFormat(), '');
}

function test_get_skeleton() :mixed{
  $skeleton = "";
  $gen = IntlDatePatternGenerator::createInstance('en_US');

  VS($gen->getSkeleton(''), '');
  VS($gen->getSkeleton('dd/MM/yyyy'), 'yyyyMMdd');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->getSkeleton(INVALID_STR);
  });
}

function test_get_base_skeleton() :mixed{
  $gen = IntlDatePatternGenerator::createInstance('en_US');

  VS($gen->getBaseSkeleton(''), '');
  VS($gen->getBaseSkeleton('dd/MM/yyyy'), 'yMd');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->getBaseSkeleton(INVALID_STR);
  });
}

function test_add_pattern() :mixed{
  $gen = IntlDatePatternGenerator::createInstance('en_US');
  VS($gen->getBestPattern('MMMMdd'), 'MMMM dd');

  $conflict = $gen->addPattern("dd-MMMM", false);
  VS($conflict, IntlDatePatternGenerator::PATTERN_NO_CONFLICT);
  VS($gen->getBestPattern('MMMMdd'), 'dd-MMMM');

  $conflict = $gen->addPattern("dd-MMMM-", false);
  VS($gen->getBestPattern('MMMMdd'), 'dd-MMMM');
  VS($conflict, IntlDatePatternGenerator::PATTERN_BASE_CONFLICT);

  $conflict = $gen->addPattern("dd-MMMM--", true);
  VS($conflict, IntlDatePatternGenerator::PATTERN_NO_CONFLICT);
  VS($gen->getBestPattern('MMMMdd'), 'dd-MMMM--');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->addPattern(INVALID_STR, true);
  });
}

function test_append_item_format() :mixed{
  $gen = IntlDatePatternGenerator::createInstance('en_US');
  VS($gen->getAppendItemFormat(ERA_FIELD), '{0} {1}');

  $gen->setAppendItemFormat(ERA_FIELD, '{1} {0}');
  VS($gen->getAppendItemFormat(ERA_FIELD), '{1} {0}');

  EXPECT_INVALID_PATTERN_FIELD(INVALID_FIELD, function () use ($gen) {
    $gen->setAppendItemFormat(INVALID_FIELD, "test");
  });

  EXPECT_INVALID_PATTERN_FIELD(FIELD_COUNT, function () use ($gen) {
    $gen->setAppendItemFormat(FIELD_COUNT, "test");
  });

  EXPECT_INVALID_PATTERN_FIELD(INVALID_FIELD, function () use ($gen) {
    $gen->getAppendItemFormat(INVALID_FIELD);
  });

  EXPECT_INVALID_PATTERN_FIELD(FIELD_COUNT, function () use ($gen) {
    $gen->getAppendItemFormat(FIELD_COUNT);
  });

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->setAppendItemFormat(ERA_FIELD, INVALID_STR);
  });
}

function test_append_item_name() :mixed{
  $gen = IntlDatePatternGenerator::createInstance('en_US');
  VS($gen->getAppendItemName(ERA_FIELD), "Era");
  $gen->setAppendItemName(ERA_FIELD, 'eras');
  VS($gen->getAppendItemName(ERA_FIELD), 'eras');

  EXPECT_INVALID_PATTERN_FIELD(INVALID_FIELD, function () use ($gen) {
    $gen->setAppendItemName(INVALID_FIELD, "test");
  });

  EXPECT_INVALID_PATTERN_FIELD(FIELD_COUNT, function () use ($gen) {
    $gen->setAppendItemName(FIELD_COUNT, "test");
  });

  EXPECT_INVALID_PATTERN_FIELD(INVALID_FIELD, function () use ($gen) {
    $gen->getAppendItemName(INVALID_FIELD);
  });

  EXPECT_INVALID_PATTERN_FIELD(FIELD_COUNT, function () use ($gen) {
    $gen->getAppendItemName(FIELD_COUNT);
  });

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->setAppendItemName(ERA_FIELD, INVALID_STR);
  });
}

function test_date_time_format() :mixed{
  $gen = IntlDatePatternGenerator::createInstance('en_US');
  //
  // Different versions of ICU will give us slightly different formats
  // for the DatePatternGenerator.  Here we choose to ignore
  // all commas, so that we accept "{1}, {0}",
  // as for example on stock ubuntu 14.04.
  //
  VS(str_replace(",", "", $gen->getDateTimeFormat()), '{1} {0}');

  $gen->setDateTimeFormat('{0} {1}');
  VS($gen->getDateTimeFormat(), '{0} {1}');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->setDateTimeFormat(INVALID_STR);
  });
}

function test_get_best_pattern() :mixed{
  $skeleton = 'yyyyMMMMddhhmm';
  $gen = IntlDatePatternGenerator::createInstance('en_US');

  VS(
    str_replace("yyyy,", "yyyy", $gen->getBestPattern($skeleton)),
    'MMMM dd, yyyy h:mm a'
  );
  VS(
    str_replace("yyyy,", "yyyy", $gen->getBestPattern($skeleton)),
    'MMMM dd, yyyy h:mm a'
  );

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->getBestPattern(INVALID_STR);
  });
}

function test_replace_field_types() :mixed{
  $gen = IntlDatePatternGenerator::createEmptyInstance();
  VS($gen->replaceFieldTypes("dd-MM-yy", 'yyyyMMMMdd'), 'dd-MMMM-yyyy');

  $newPattern = $gen->replaceFieldTypes("dd-MM-yy", 'yyyyMMMMdd');
  VS($newPattern, 'dd-MMMM-yyyy');

  $newPattern = $gen->replaceFieldTypes('dd-MM-yy hh:mm', 'yyyyMMMMddhm');
  VS($newPattern, 'dd-MMMM-yyyy hh:mm');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->replaceFieldTypes(INVALID_STR, 'yyyyMMMMddhm');
  });

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->replaceFieldTypes('dd-MM-yy hh:mm', INVALID_STR);
  });
}

function test_get_skeletons() :mixed{
  $gen = IntlDatePatternGenerator::createEmptyInstance();

  $skeletons = $gen->getSkeletons();
  VS($skeletons->valid(), false);

  $gen->addPattern('dd-MM-yy', false);

  $skeletons = $gen->getSkeletons();
  VS($skeletons->next(), 'yyMMdd');
  VS($skeletons->next(), null);
}

function test_get_pattern_for_skeleton() :mixed{
  $gen = IntlDatePatternGenerator::createEmptyInstance();
  $pattern = $gen->getPatternForSkeleton('yyMMdd');
  VS($pattern, '');

  $gen->addPattern('dd-MM-yy', false);

  $pattern = $gen->getPatternForSkeleton('yyMMdd');
  VS($pattern, 'dd-MM-yy');

  $pattern = $gen->getPatternForSkeleton('yyMMd');
  VS($pattern, '');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->getPatternForSkeleton(INVALID_STR);
  });
}

function test_get_base_skeletons() :mixed{
  $gen = IntlDatePatternGenerator::createEmptyInstance();
  $skeletons = $gen->getBaseSkeletons();
  VS($skeletons->valid(), false);

  $gen->addPattern('dd-MM-yy', false);

  $skeletons = $gen->getBaseSkeletons();
  VS($skeletons->next(), 'yMd');
  VS($skeletons->next(), null);
}

function test_decimal() :mixed{
  $gen = IntlDatePatternGenerator::createInstance('en_US');
  VS($gen->getDecimal(), '.');

  $gen->setDecimal(',');
  VS($gen->getDecimal(), ',');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->setDecimal(INVALID_STR);
  });
}

function test_get_error() :mixed{
  $gen = IntlDatePatternGenerator::createInstance('en_US');
  VS($gen->getErrorCode(), 0);
  VS($gen->getErrorMessage(), 'U_ZERO_ERROR');

  EXPECT_INVALID_STR($gen, function () use ($gen) {
    $gen->setDateTimeFormat(INVALID_STR);
  });
}


<<__EntryPoint>>
function main_date_pattern_gen() :mixed{
$tests = vec[
  'test_create_instance_with_no_locale_fails',
  'test_create_instance_with_null_locale_fails',
  'test_create_instances_with_different_locales',
  'test_create_empty_instance',
  'test_get_skeleton',
  'test_get_base_skeleton',
  'test_add_pattern',
  'test_append_item_format',
  'test_append_item_name',
  'test_date_time_format',
  'test_get_best_pattern',
  'test_replace_field_types',
  'test_get_skeletons',
  'test_get_pattern_for_skeleton',
  'test_get_base_skeletons',
  'test_decimal',
  'test_get_error'
];

foreach ($tests as $test) {
  echo "Running $test\n";
  $test();
}
}
