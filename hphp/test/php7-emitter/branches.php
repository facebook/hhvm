<?php

if (true) {
  echo "hello\n";
} else {
  echo "wat\n";
}

if (2 > 10) {
  echo "no\n";
} else if (2 > 5) {
  echo "no\n";
} else if ("foo" === false) {
  echo "no\n";
} else if (1 < 2) {
  echo "yes\n";
} else if (0 <= 2) {
  echo "doesn't get here\n";
}

if (null == 2) {
  echo "no\n";
} else {
  echo "yes\n";
}
