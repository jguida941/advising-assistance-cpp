# Advising assistance: CS499 review

This directory makes the Milestone One review reproducible. Each report identifies the original source, supplies a small input, distinguishes observed behavior from a proposed requirement, and records the limits of the example.

## Cases

| Report | Tracking issue | State |
|---|---|---|
| [ADVISER-CSV](cases/ADVISER-CSV.md) | [Issue #2](https://github.com/jguida941/advising-assistance-cpp/issues/2) | Expected behavior review pending |
| [ADVISER-GRAPH](cases/ADVISER-GRAPH.md) | [Issue #3](https://github.com/jguida941/advising-assistance-cpp/issues/3) | Expected behavior review pending |

The [work index](work-slices.toml) records the actual foundation branch and separately names planned future branches. An issue being open or an example exiting successfully does not establish that an enhancement is implemented.

## Run from this repository

```sh
python3 examples/cs499/run_examples.py csv
python3 examples/cs499/run_examples.py graph
```

Read each case's prerequisites first. Source hashes in the case JSON identify the reviewed files. If those files or the example change, refresh the observations instead of presenting old output as current.

The demonstrations compile and execute the real native catalog code. They do not launch Qt. Quoted CSV support and prerequisite scheduling are proposed enhancements; the existing importer does not promise a topological schedule.

## Review together

1. Read the input, expected result, actual result and limitation.
2. Run the example and explain the output in your own words.
3. Decide whether the expectation comes from an existing requirement or is a new enhancement.
4. Record that decision before implementing a product change.
5. Implement one bounded change on its own branch, add an appropriate regression test, and open a focused PR with the before/after evidence.

This foundation adds examples and review records. It preserves the selected product implementation. Findings remain pending Justin's review; no human acceptance or product repair is claimed.

## Documentation

- [Review requirements](requirements.md)
- [Architecture decision](../adrs/0001-coursework-review.md)
- [Original reviewed source](https://github.com/jguida941/advising-assistance-cpp/tree/7b2b0c4986e76adac8bb37fcdf45806c3fe30843)

Shared review commands are maintained outside this application repository. These ordinary examples can be run without them. AI assisted in preparing the reports and examples; the records identify the remaining human review.

## Received foundation and planned organization work

[Foundation PR #4](https://github.com/jguida941/advising-assistance-cpp/pull/4) was merged into `master` at `7b5b2159ac154bac1f9e8d47b62a067280922c31`. The [receiving record](receipts/review-foundation-merged.json) retains the observed PR and default-branch facts. The foundation row is delivered in that integration sense; human review of the explanations, proposed behavior and remaining design is still pending.

The planned organization work in [issue #5](https://github.com/jguida941/advising-assistance-cpp/issues/5) keeps the catalog, CLI and GUI source folders. It first verifies a clean source build, then removes generated build output from tracking and adds ignore rules while preserving local files. IDE settings need separate inspection. This plan has not been implemented, and no clean Qt build is claimed. Its named branch is planned, not created.

The work index keeps the last selected foundation row until a new target slice is deliberately selected. The application enhancements above remain separate pending cases. Status fields record observations, not a live GitHub query; follow the linked PR for newer publication state.

[Issue #6](https://github.com/jguida941/advising-assistance-cpp/issues/6) and branch `cs499/issue-6-receiving-records` track this documentation-only state correction. Its review PR will be recorded in the same work index. No application, test, CI or layout change is included.
