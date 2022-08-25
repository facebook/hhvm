ARG BASE_IMAGE
FROM $BASE_IMAGE

WORKDIR /watchman
RUN ./install-system-packages.sh

# Clean up the temporary build artifacts so the image is smaller.
RUN ./autogen.sh && rm -rf /tmp/fbcode_builder_getdeps*
