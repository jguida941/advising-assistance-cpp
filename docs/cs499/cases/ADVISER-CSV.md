# Define CSV quoting support and preserve complete course titles

Case: ADVISER-CSV  
Status: need-human-review — Justin has not yet approved the interpretation or desired change.  
Classification: import enhancement

## Problem and requirement

The current comma splitter treats a comma inside quotes as a separator. The remaining title text reaches the prerequisite parser, which discards it with a warning.

Quoted CSV fields are a proposed supported format; the original grammar was not explicit.

## Run the example

From the repository root:

```sh
python3 examples/cs499/run_examples.py csv
```

Python 3 and g++ with C++20 support. Compilation uses the actual catalog source; all input files and binaries are temporary.

Exit 0 means the observation completed. It does not mean a proposed enhancement is implemented. Product source is unchanged by the example.

## Input

One row: CSCI100,"Data, Structures". Compare it with CSCI100,Data Structures.

## Expected

If quoted CSV fields are supported, the stored title is exactly Data, Structures. The ordinary unquoted title must still load correctly.

## Observed at the reviewed revision

The current loader accepts the quoted row but stores the title as "Data and reports an invalid prerequisite token for the remaining text. The ordinary row loads with the full title Data Structures.

Source revision: `7b2b0c4986e76adac8bb37fcdf45806c3fe30843`. The neighboring JSON records the exact product-file hashes used for this observation. The saved output is [`ADVISER-CSV.txt`](../observations/ADVISER-CSV.txt).

## Proposed work

Specify the supported CSV syntax, then use a small suitable parser or a carefully scoped parser change. Decide how malformed rows affect browseability and scheduling.

Acceptance to discuss before implementation:

- Quoted commas remain inside the course title when that format is selected.
- Ordinary rows, normalized IDs and valid prerequisites continue to load.
- Malformed quoted fields receive an explicit diagnostic.
- The chosen grammar and invalid-row policy are documented and covered by examples.

## Limits

This is an observed parsing limitation against a proposed format, not proof that the original assignment promised full CSV quoting. Only the actual catalog core is executed; Qt is not launched.

## Source pointers

- https://github.com/jguida941/advising-assistance-cpp/blob/7b2b0c4986e76adac8bb37fcdf45806c3fe30843/src/catalog/catalog.cpp#L149-L171

## Human review

Justin should review the requirement, run or inspect the demonstration, and decide whether the proposed change fits the course enhancement. That review remains pending. No fix, agreement or completed enhancement is claimed here.
