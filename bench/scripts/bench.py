"""
    This script will run all benchmarks across the entire workspace.
"""

import datetime
import os
import subprocess

def main():
    # Move to the root folder of the project
    os.chdir(os.environ["BUILD_WORKSPACE_DIRECTORY"] + "/bench")

    # If the file doesn't exist, create it and set the headers
    filename = "bench_results.csv"
    headers = ["datetime", "bench_set", "function_name", "test_name", "mean", "variance", "stdev", "coef_of_variance"]
    if not os.path.isfile(filename):
        with open(filename, 'w') as outfile:
            outfile.write(",".join(headers) + "\n")

    launchtime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # Query bazel for all build targets
    p = subprocess.run(["bazel", "query", "//..."], capture_output=True)
    if p.returncode != 0:
        print("Error encountered!")
        print(p.stderr)
        exit()

    # Pick all build targets of the form //[my_package]:benchmark
    target_str = ":benchmark"
    lines = [x.decode('UTF-8') for x in p.stdout.split()]
    bench_targets = [x for x in lines if len(x) > len(target_str) and x[-len(target_str):] == target_str]

    # Run those
    print("\x1b[1;31mFound", len(bench_targets), "benchmark targets to run.\x1b[0m")
    for target in bench_targets:
        p = subprocess.run(["bazel", "run", target, "--", launchtime, filename], capture_output=False)

if __name__ == "__main__":
    main()