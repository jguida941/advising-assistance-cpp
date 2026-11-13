# CS499 review requirements

Status: implemented as a review foundation; expected behavior decisions remain pending.

1. Every finding identifies source files and revision, input, expected result, actual result, proposed change, acceptance checks and execution limits.
2. An observation executes the actual selected application code. Any temporary comparison is labeled explicitly.
3. Existing behavior that satisfies the current contract is described accurately. An enhancement is not mislabeled as a confirmed original defect.
4. A real issue, branch and PR track the foundation; planned repair branches are not represented as created.
5. Justin reviews the explanation and intended behavior before a repair is accepted. A recorded observation is not human approval.
6. Product tests and examples remain runnable in this repository. Shared tooling remains external.
7. Examples contain ordinary product concepts and comments, suitable for an instructor. Private development records are excluded from this repository's review materials.
8. This increment preserves product behavior. Further enhancements have separate issues and review.

Reviewed baseline: `7b2b0c4986e76adac8bb37fcdf45806c3fe30843`.

## Evidence limits

The demonstrations compile and execute the real native catalog code. They do not launch Qt. Quoted CSV support and prerequisite scheduling are proposed enhancements; the existing importer does not promise a topological schedule.

The compact work index is a record of correspondence, not a live GitHub status service. Remote state must be observed before recording merging or delivery. No merge has been performed for this foundation.
