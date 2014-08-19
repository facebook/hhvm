<?hh
type Status = string;
class Statuses {
  const Status FATAL = "FATAL";
  const Status UNKNOWN = "UNKNOWN STATUS";
  const Status PASS = ".";
  const Status FAIL = "F";
  const Status ERROR = "E";
  const Status INCOMPLETE = "I";
  const Status SKIP = "S";
  const Status RISKY = "R";
  const Status TIMEOUT = "TIMEOUT";
  const Status BLACKLIST = "BLACKLIST";
  const Status CLOWNY = "CLOWNY";
  const Status WARNING = "WARNING";
}
