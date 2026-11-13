# Add prerequisite-cycle analysis and valid course ordering

Case: ADVISER-GRAPH  
Status: need-human-review — Justin has not yet approved the interpretation or desired change.  
Classification: new algorithmic feature

## Problem and requirement

Reference existence and orderability are different properties. Both references in a cycle can exist, while neither course can be completed first. The current alphabetic display must not be described as a computed prerequisite order.

Scheduling and cycle analysis are planned enhancements, not existing behavior promises.

## Run the example

From the repository root:

```sh
python3 examples/cs499/run_examples.py graph
```

Python 3 and g++ with C++20 support. This example executes import behavior only; its proposed scheduling outcomes are explanations, not implemented results.

Exit 0 means the observation completed. It does not mean a proposed enhancement is implemented. Product source is unchanged by the example.

## Input

CSCI100 requires CSCI200, and CSCI200 requires CSCI100. Compare with a two-course chain in which CSCI200 requires CSCI100.

## Expected

The proposed scheduling feature reports that the cycle has no complete valid order. For the ordinary chain, a complete order contains each course once and puts CSCI100 before CSCI200.

## Observed at the reviewed revision

The current loader accepts both catalogs. The cycle has zero warnings and zero missing prerequisites because both IDs exist. The current API loads, looks up and lists courses; it does not produce a prerequisite schedule.

Source revision: `7b2b0c4986e76adac8bb37fcdf45806c3fe30843`. The neighboring JSON records the exact product-file hashes used for this observation. The saved output is [`ADVISER-GRAPH.txt`](../observations/ADVISER-GRAPH.txt).

## Proposed work

Build prerequisite-to-dependent adjacency lists and indegree counts in the shared core. Use a queue-based topological order. Report failure if not every course is processed, then display the result in Qt.

Acceptance to discuss before implementation:

- A successful order contains every course exactly once, with each prerequisite before its dependent.
- Chains, branches, independent courses, self-cycles and multi-course cycles have explicit examples.
- Missing prerequisites and malformed import data cannot silently become a complete valid schedule.
- Unprocessed downstream courses are not all mislabeled as cycle members.
- Graph work remains in the shared core; Qt presents its result.

## Limits

Accepting a catalog for browsing is not automatically wrong. Justin must decide whether invalid scheduling data remains browseable. A new algorithm is required; no existing scheduling implementation was tested or repaired.

## Source pointers

- https://github.com/jguida941/advising-assistance-cpp/blob/7b2b0c4986e76adac8bb37fcdf45806c3fe30843/src/catalog/catalog.cpp#L204-L230
- https://github.com/jguida941/advising-assistance-cpp/blob/7b2b0c4986e76adac8bb37fcdf45806c3fe30843/include/catalog/catalog.hpp#L23-L41

## Human review

Justin should review the requirement, run or inspect the demonstration, and decide whether the proposed change fits the course enhancement. That review remains pending. No fix, agreement or completed enhancement is claimed here.
