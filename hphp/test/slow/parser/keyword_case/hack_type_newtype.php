<?hh

TYPE T = string;
NEWTYPE NT = int;

function typed(T $_a, NT $_b) {
  ECHO "I WILL DO ANYTHING FOR TYPES\n";
}

typed('a', 1);
try {
  typed(1, 'a');
} catch (Error $_) {
  ECHO "BUT I WON'T DO THAT\n";
}
