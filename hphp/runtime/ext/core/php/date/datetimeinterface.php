<?hh

/* Type hints are missing as any classes that implements this interface must
 * specify types, meaning that no classes written in PHP can implement this
 * interface if type hints are included.
 */
interface DateTimeInterface {
  public function diff(
    DateTimeInterface $datetime2,
    bool $absolute = false,
  ): mixed;
  public function format(string $format): string;
  public function getOffset(): mixed;
  public function getTimestamp()[]: int;
  public function getTimezone(): mixed;
}
