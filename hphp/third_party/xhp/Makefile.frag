XHP_SHARED_DEPENDENCIES = $(srcdir)/xhp/libxhp.a
XHP_SHARED_LIBADD := ${XHP_SHARED_LIBADD}
$(srcdir)/ext.cpp: $(srcdir)/xhp/libxhp.a
$(srcdir)/xhp/libxhp.a: FORCE
	$(MAKE) $(MFLAGS) -C $(srcdir)/xhp libxhp.a

FORCE:
