FROM ghcr.io/xtruder/nix-devcontainer:v1
RUN echo 'extra-sandbox-paths = /nix/var/cache/ccache' | sudo tee -a /etc/nix/nix.conf
RUN sudo mkdir -m0770 -p /nix/var/cache/ccache
RUN sudo chown --reference=/nix/store /nix/var/cache/ccache
