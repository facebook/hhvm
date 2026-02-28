---
description: How to preview the doc site locally
---

# Local Preview

Thrift content is built using [Docusaurus](https://docusaurus.io). For simple edits and suggestions, you can use the <strong>Edit this page</strong> or the <strong>Add new page</strong> button at the end of each page to submit your edit.

For more complex edits and suggestions, you can clone the repo locally, make your edits, preview, and submit a pull request.

## Install Website from GitHub
Prereq: Make sure you have `yarn` and `node.js` installed on your local machine.

From a commandline or SSH:
1. Run `git clone https://github.com/facebook/fbthrift` to clone the fbThrift project
1. cd into `cd fbthrift/thrift/website`
1. Run `yarn install` to install website packages and any dependancies. You only need to do this once
1. Run `yarn start` to start the webserver
1. To preview the website, point your web browser to: `localhost:3000`
