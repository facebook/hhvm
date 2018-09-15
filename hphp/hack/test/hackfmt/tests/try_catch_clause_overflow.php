<?hh

try {
  foo();
} catch (LongishExceptionType $exception_with_very_long_name_causing_line_breaks) {
  handleLongishExceptionType($exception_with_very_long_name_causing_line_breaks);
} catch (EvenMoreWordilyNamedExceptionType $exception_with_very_long_name_causing_line_breaks) {
  handleEvenMoreWordilyNamedExceptionType($exception_with_very_long_name_causing_line_breaks);
}
