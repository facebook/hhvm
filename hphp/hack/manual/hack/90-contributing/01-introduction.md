# Introduction

## Prerequisites

This documentation site is built with [Docusaurus](https://docusaurus.io/). To run the site locally for development and testing, ensure [Node.js](https://nodejs.org/en/) (v20.0 or above) and [Yarn](https://yarnpkg.com/) are installed.

<FbInfo>
As a Meta employee, to run the site on an **On Demand** or a **Dev Server**, follow the instructions on the [Static Docs prerequisites page](https://www.internalfb.com/intern/staticdocs/staticdocs/setup/2-prequisites/), which also describes how to set up proxying. Any updates to the documentation in the repository will be reflected on the [internal Static Docs](https://www.internalfb.com/intern/staticdocs/hack/) site through [automated periodic deployments](https://www.internalfb.com/intern/staticdocs/staticdocs/setup/4-configerator/#periodic-deployment)
</FbInfo>

## Running locally

1. Navigate to the `website/` directory and run `yarn install` to install all the required dependencies.

2. Next, to run the site locally, run `yarn start`.

3. This will start a local server on port 3000. You can view the site at [http://localhost:3000](http://localhost:3000).

<FbInfo>
The documentation site allows for creation of internal paragraphs/blocks that are only visible to Meta employees. To view these parts of the documentation, you must be on a Meta network or VPN. For local testing, running `yarn start-fb` will switch the local server into a mode that will allow you to view the internal-only parts of the documentation.
</FbInfo>

Whilst `yarn start` is running, any changes made to the documentation will be published to the local instance automatically, allowing for a fast feedback loop as you make updates.

## Running an old version

If you want to see old versions of the docs (from before our migration to Docusaurus in 2025), we provided [regular Docker images of the old documentation site](https://ghcr.io/hhvm/user-documentation) on GHCR (previously
on [Docker Hub](https://hub.docker.com/r/hhvm/user-documentation/tags)).

These images are not suitable for development, but are useful if you are working with
an old HHVM/Hack version.

1. Install [Docker](https://docs.docker.com/engine/installation/).
2. Start a container with: `docker run -p 8080:80 -d ghcr.io/hhvm/user-documentation`.
3. You can then access a local copy of the documentation at [http://localhost:8080](http://localhost:8080).
