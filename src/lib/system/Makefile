
PROJECT_ROOT = ../../..

HPHP = $(PROJECT_ROOT)/src/hphp/hphp
PHP_FILES = $(wildcard */*.php)

all:
	$(HPHP) -t cpp -f sys --input-dir . -i $(PHP_FILES) -o gen 1>/dev/null
	$(V)find . -name "*.h" -or -name "*.cpp" | xargs svn add 2>&1 | grep -v "is already under version control" || echo "lib/system files re-generated"
	$(V)php $(PROJECT_ROOT)/src/license.php
