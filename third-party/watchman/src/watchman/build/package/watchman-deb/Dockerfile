ARG BASE_IMAGE
FROM $BASE_IMAGE

WORKDIR /watchman

ENV BUILT="/watchman/built"

ADD make-deb.sh /watchman/build/make-deb.sh
ADD DEBIAN /watchman/watchman/build/package/DEBIAN

# TODO: make a debugsymbols package
RUN strip built/bin/*

RUN /watchman/build/make-deb.sh
