# Vagrant VMs for Validating Watchman Builds

Watchman uses GitHub Actions for CI, but sometimes it's useful to have
reproducible, bare environments representative of where people
typically attempt to build Watchman.

This Vagrantfile provides some provisioned VMs with the minimal set of
dependencies required to build and run Watchman.

None of the VMs will autostart, so they must be specified explicitly
to `vagrant up`. When running `vagrant up` or `vagrant provision`,
ensure the `WATCHMAN_SOURCE` environment variable contains a path to
the Watchman source directory.
