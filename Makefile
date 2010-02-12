
fast_tests:
	$(MAKE) -C src
	cd src && test/test "" "" QuickTests
	cd src && test/test "" "" TestExt
	cd src && test/test TestCodeRunEval

slow_tests:
	$(MAKE) -C src
	cd src && time test/test TestCodeRun
	cd src && time test/test TestServer

clobber:
	$(MAKE) -C src clobber
	$(MAKE) -C facebook clobber

clean: clobber
