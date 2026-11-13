# ADR-0001: Reproducible coursework review

Status: proposed in the foundation PR; pending owner review.
Date: 2026-11-13

## Context

The code review needs examples a reader can rerun and distinguish from proposed enhancements. Keeping its only record in chat or a recording script makes source changes and expected behavior decisions hard to trace.

## Decision

Keep ordinary case reports, small demonstrations, observed output and a compact work index in this repository. Maintain shared review tooling in a separate review repository, accessed through explicit checkout paths. Application builds and examples do not depend on that shared repository.

The current product code remains the baseline. Product enhancements use separate issues, branches and PRs after the intended behavior is reviewed. Generated reports identify their source and observation rather than implying a completed repair.

## Consequences

The instructor can inspect a short problem report and execute its example. Product-specific requirements stay with the code. Cross-project tooling has one maintenance location. Expected behavior and human understanding still require review; automation does not supply those decisions.

## Alternatives

Duplicating shared review tooling in every application would create drift. Housing it in one application would make that application an accidental dependency for the others. Keeping only an external document would weaken reproducibility.

## References

- [Review guide](../cs499/README.md)
- [Foundation issue](https://github.com/jguida941/advising-assistance-cpp/issues/1)
