# Object Model Maintainer Guide

This directory contains the canonical Thrift Object Model specification.

## Making Changes

1. Edit [`index.md`](./index.md).
2. Preserve the document's existing terminology, structure, and editorial style.
3. Keep normative requirements and semantic examples in `index.md`; keep release workflow and other maintainer notes in this `README.md`.
4. If the published specification changes, update the changelog in [`index.md`](./index.md) in the same diff.
5. Before sending changes for review, run `arc f` and `arc lint` on the touched files from the repository root.

Significant or potentially breaking changes should receive review from Thrift maintainers before landing.

## Release Checklist

Landing a change to [`index.md`](./index.md) updates the canonical Object Model.

1. Decide the required version bump using the `Versioning` section in [`index.md`](./index.md).
2. Add a changelog entry to [`index.md`](./index.md).
3. Ensure terminology, examples, and cross-references remain consistent.
4. Get review appropriate for the scope of the change.
5. Preview the rendered docs locally.
6. Land the change and announce notable releases to downstream consumers as appropriate.

## Preview Locally

From `xplat/thrift/website`:

```bash
yarn install
yarn start-fb
```
