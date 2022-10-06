FROM ghcr.io/xtruder/nix-devcontainer:v1
RUN echo 'extra-sandbox-paths = /nix/var/cache/ccache' >> /etc/nix/nix.conf
RUN mkdir -m0770 -p /nix/var/cache/ccache
RUN chown --reference=/nix/store /nix/var/cache/ccache
