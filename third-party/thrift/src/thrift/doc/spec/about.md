---
state: experimental
---

# Specification Format

## Key Words and Terms

Keywords are used to indicate a precise, formulaic meaning. Keywords are denoted in bold, for example: **may**, **must not**, **should**.

Key terms have a special meaning or interpretation to Thrift, and are denoted with italics, for example: *native type*, *underlying type*.

## Metadata

All spec files **must** start with metadata about the page, which includes:

- `state` - The current state of the page. **Must** be one of
  - `draft` - Spec in progress.
  - `experimental` - Ready for testing.
  - `beta` - Sufficient tests and documentation for public review and feedback.
  - `released` - Complete, well tested and reviewed.

For example, every page should start out with:

```yaml
---
state: draft
---
```
