XHP_SHARED_DEPENDENCIES = xhp/libxhp.a
XHP_SHARED_LIBADD := xhp/libxhp.a ${XHP_SHARED_LIBADD}
$(srcdir)/ext.cpp: xhp/libxhp.a
xhp/libxhp.a: FORCE
	$(MAKE) $(MFLAGS) -C xhp libxhp.a

FORCE:
