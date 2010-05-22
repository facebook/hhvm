
PROJECT_ROOT = ../..

HPHP = $(PROJECT_ROOT)/src/hphp/hphp
PHP_FILES = $(wildcard */*.php)

all:
	$(HPHP) -t cpp -f sys --input-dir . -i $(PHP_FILES) -o gen 1>/dev/null
	$(V)php $(PROJECT_ROOT)/bin/license.php
