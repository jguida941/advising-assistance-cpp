#!/usr/bin/env python3
"""Compile the actual catalog core and display small import examples."""
import argparse
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
EXAMPLE = Path(__file__).with_name("CatalogExamples.cpp")
CASES = {
    "csv": [
        ("Quoted course title", 'CSCI100,"Data, Structures"\n',
         "Proposed CSV requirement: preserve Data, Structures as one complete title."),
        ("Ordinary title comparison", "CSCI100,Data Structures\n",
         "Existing behavior: accept the course with title Data Structures.")
    ],
    "graph": [
        ("Two-course cycle", "CSCI100,Course One,CSCI200\nCSCI200,Course Two,CSCI100\n",
         "Proposed scheduling requirement: report the cycle and do not return a complete valid schedule."),
        ("Ordinary prerequisite chain", "CSCI200,Advanced Course,CSCI100\nCSCI100,Intro Course\n",
         "Existing import behavior: accept both courses with no missing prerequisite. A future schedule should put CSCI100 before CSCI200.")
    ]
}


def execute(command, timeout=45):
    result = subprocess.run(command, text=True, capture_output=True, timeout=timeout)
    if result.returncode:
        raise RuntimeError((result.stderr or result.stdout).strip())
    return result.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("example", choices=sorted(CASES))
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="catalog-example-") as temporary:
        folder = Path(temporary)
        binary = folder / "catalog-example"
        execute(["g++", "-std=c++20", "-Wall", "-Wextra", "-pedantic", "-I", str(ROOT / "include"),
                 str(ROOT / "src/catalog/catalog.cpp"), str(EXAMPLE), "-o", str(binary)])
        for index, (title, data, expected) in enumerate(CASES[args.example]):
            fixture = folder / ("catalog-" + str(index) + ".csv")
            fixture.write_text(data, encoding="utf-8")
            print(title.upper(), flush=True)
            print("Input:\n" + data.rstrip(), flush=True)
            print("Expected: " + expected, flush=True)
            print(execute([str(binary), str(fixture)], timeout=10), end="", flush=True)
            print()
    if args.example == "csv":
        print("Limit: quoted-field support is a proposed enhancement; the original assignment's CSV grammar was not explicit.")
    else:
        print("Limit: this example calls loading and lookup. The proposed scheduling expectations above are not computed by this example.")
    print("Actual native catalog code was executed. Qt was not launched and no project source was edited.")
    print("Observation completed. Exit 0 does not mean the proposed enhancement exists.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as error:
        print("Example could not complete: " + str(error), file=sys.stderr)
        raise SystemExit(2)
