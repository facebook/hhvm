#
# Full build image
#
FROM martenseemann/quic-network-simulator-endpoint:latest

ENV TZ=America/Los_Angeles
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update

RUN apt-get --yes --fix-missing update

# Get and build proxygen with HTTP/3 support
RUN apt-get install --yes wget net-tools iputils-ping tcpdump ethtool iperf git sudo cmake python3 libssl-dev m4 zlib1g-dev gcc-10 g++-10
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-10 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-10 100
RUN mkdir proxygen
COPY . /proxygen
RUN cd proxygen && ./getdeps.sh --no-tests --allow-system-packages
RUN ldd /tmp/fbcode_builder_getdeps-ZproxygenZbuildZfbcode_builder-root/build/proxygen/proxygen/httpserver/hq | grep "=> /" | awk '{print $3}' > libs.txt
RUN tar cvf libs.tar --dereference --files-from=libs.txt

#
# Minimal image
#
FROM martenseemann/quic-network-simulator-endpoint:latest
# copy run script
COPY proxygen/httpserver/samples/hq/quic-interop/run_endpoint.sh .
RUN chmod +x run_endpoint.sh

# Copy HQ
COPY --from=0 /tmp/fbcode_builder_getdeps-ZproxygenZbuildZfbcode_builder-root/build/proxygen/proxygen/httpserver/hq /proxygen/_build/proxygen/bin/hq
# Copy shared libs
COPY --from=0 libs.tar /
RUN tar xvf libs.tar
RUN rm libs.tar
# Create the logs directory
RUN mkdir /logs

ENTRYPOINT [ "./run_endpoint.sh" ]
