ARG FEDORA_VERSION
FROM fedora:$FEDORA_VERSION

RUN dnf install -y gcc g++ git openssl-devel rpmdevtools

RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# Avoid build output from Watchman's build getting buffered in large
# chunks, which makes debugging progress tough.
ENV PYTHONUNBUFFERED=1
