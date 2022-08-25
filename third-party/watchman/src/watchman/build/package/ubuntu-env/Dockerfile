ARG UBUNTU_VERSION
FROM ubuntu:$UBUNTU_VERSION

# https://serverfault.com/a/1016972 to ensure installing tzdata does not
# result in a prompt that hangs forever.
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get -y update
RUN apt-get -y install python3 gcc g++ git libssl-dev curl
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y

RUN git clone https://github.com/facebook/watchman.git
ENV PATH="/root/.cargo/bin:${PATH}"

ENV PYTHONUNBUFFERED=1

# For debugging container processes:
RUN apt-get -y install strace
