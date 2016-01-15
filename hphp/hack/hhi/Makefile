.PHONY: hhi.tar.gz
hhi.tar.gz:
	# equivalent to tar -czf, but deterministic output
	find . -print0 | LC_ALL=C sort -z | tar --null \
		--mtime='1970-01-01 00:00Z' \
		--owner=root --group=root \
		--numeric-owner \
		-T - --no-recursion \
		-c | gzip -n > ../bin/hhi.tar.gz
