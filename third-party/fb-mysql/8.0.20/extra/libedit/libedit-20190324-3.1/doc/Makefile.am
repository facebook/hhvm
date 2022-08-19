
EL_MANS = editline.3 editline.7 editrc.5

man_MANS = $(EL_MANS)


EL_MAN_LINKS = \
		el_deletestr.3 \
		el_end.3 \
		el_get.3 \
		el_getc.3 \
		el_gets.3 \
		el_init.3 \
		el_init_fd.3 \
		el_insertstr.3 \
		el_line.3 \
		el_parse.3 \
		el_push.3 \
		el_reset.3 \
		el_resize.3 \
		el_set.3 \
		el_source.3 \
		history.3 \
		history_end.3 \
		history_init.3 \
		tok_end.3 \
		tok_init.3 \
		tok_line.3 \
		tok_reset.3 \
		tok_str.3

EL_MAN_LINKS += \
		el_wdeletestr.3 \
		el_wget.3 \
		el_wgetc.3 \
		el_wgets.3 \
		el_winsertstr.3 \
		el_wline.3 \
		el_wparse.3 \
		el_wpush.3 \
		el_wset.3 \
		history_w.3 \
		history_wend.3 \
		history_winit.3 \
		tok_wend.3 \
		tok_winit.3 \
		tok_wline.3 \
		tok_wreset.3 \
		tok_wstr.3


install-data-hook: $(EL_MAN_LINKS)

$(EL_MAN_LINKS):
	(cd $(DESTDIR)$(man3dir) && rm -f $@ && $(LN_S) editline.3 $@)

$(EL_MANS):
	@if test "$(MANTYPE)" = "mdoc"; then\
		cp $(srcdir)/$@.roff $@;\
				else\
		$(AWK) -f $(srcdir)/mdoc2man.awk $(srcdir)/$@.roff > $@ || rm -f $@;\
	fi;

uninstall-local:
	(cd $(DESTDIR)$(man3dir) && rm -f $(EL_MAN_LINKS))

CLEANFILES = $(EL_MANS)
EXTRA_DIST = editline.3.roff editline.7.roff editrc.5.roff mdoc2man.awk

changelog.txt: ../ChangeLog
	@sed 's/@/ (at)/g; s/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g;' $(srcdir)/$< > $@;


