
PROJECT_ROOT = ../..

include $(PROJECT_ROOT)/src/dirs.mk

PHP_FILES = $(wildcard classes/*.php) $(wildcard globals/*.php)

PHP := $(wildcard /usr/local/hphpi/bin/hphpi)
PHP := $(if $(PHP),$(PHP) -v Preg.ErrorLog=false,php)
all:
	$(HPHP) --opts=none -t cpp -f sys --input-dir . -i $(PHP_FILES) -o gen 1>/dev/null
	$(V)$(PHP) $(PROJECT_ROOT)/bin/license.php
	$(V)$(PHP) $(PROJECT_ROOT)/bin/ext_injection.php
