ARG UBUNTU_VERSION
FROM ubuntu:$UBUNTU_VERSION

# https://serverfault.com/a/1016972 to ensure installing tzdata does
# not result in a prompt that hangs forever.
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get -y update
RUN apt-get -y install python3 gcc g++ libssl-dev curl sudo

# Ubuntu 18.04 has an older version of Git, which causes actions/checkout@v3
# to check out the repository with REST, breaking version number generation.
RUN apt-get -y install software-properties-common
RUN add-apt-repository -y ppa:git-core/ppa
RUN apt-get -y install git

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# Avoid build output from Watchman's build getting buffered in large
# chunks, which makes debugging progress tough.
ENV PYTHONUNBUFFERED=1
