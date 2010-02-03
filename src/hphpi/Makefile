
PROJECT_ROOT = ../..

HPHP = $(PROJECT_ROOT)/src/hphp/hphp
PHP_FILES = hphpi.php

all: hphpi

hphpi: $(HPHP) $(PHP_FILES) hphpi_build.hdf
	@echo "Compiling hphpi..."
	@$(HPHP) -t cpp -f exe --input-dir . -i $(PHP_FILES) -o gen \
		--config hphpi_build.hdf --log=1
	@mv gen/program hphpi

clobber:
	@rm -rf gen hphpi
