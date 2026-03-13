import argparse
import os
from pathlib import Path
import subprocess


class bcolors:
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"


class ExitCodes:
    CALL_ERROR = 1
    FILE_NOT_FOUND_ERROR = 2
    SCANNING_ERROR = 3
    PARSING_ERROR = 4
    CODE_GEN_ERROR = 5


parser = argparse.ArgumentParser(
    prog="CobaltTestBench", description="Run test suite for Cobalt"
)

parser.add_argument("-dir", "--dir", default="")
parser.add_argument("--print_stderr", action="store_true", default=False)

cur_dir = Path(__file__).parent

args = parser.parse_args()

tests_dir: Path

if len(args.dir) == 0:
    tests_dir = cur_dir / "testfiles/passing"
else:
    tests_dir = Path(args.dir)

tests_dir = tests_dir.absolute()

print(f"{bcolors.HEADER}Compiling Cobalt{bcolors.ENDC}")
res = subprocess.run(
    "cmake --build .",
    cwd=cur_dir.parent / "build",
    capture_output=True,
    shell=True,
)
if res.returncode != 0:
    print(f"{bcolors.FAIL}Error compiling Cobalt{bcolors.ENDC}")
    print(res.stderr.decode())
    exit(1)

c_files = []

for root, dirs, files in os.walk(tests_dir):
    for file_name in files:
        file = Path(root) / file_name
        if file.suffix != ".c":
            print(
                f"{bcolors.WARNING} Ignoring file {file} - must have .c extension {bcolors.ENDC}"
            )
            continue

        c_files.append(file)

c_files.sort()

total = len(c_files)
correct = 0
assembly_files = []

for file in c_files:
    print(f"{bcolors.HEADER} Running test for {str(file)} {bcolors.ENDC}")

    gcc_compilation_res = subprocess.run(
        f"gcc {file} -o gcc_exec.out",
        cwd=cur_dir,
        shell=True,
        capture_output=True,
    )

    compilation_should_fail = False
    if gcc_compilation_res.returncode != 0:
        compilation_should_fail = True

    assembly_file = file.with_suffix(".s")
    res = subprocess.run(
        f"./cobalt {file} {assembly_file}",
        cwd=cur_dir.parent / "build",
        shell=True,
        capture_output=True,
    )
    passing = True
    if not compilation_should_fail and res.returncode != 0:
        if res.returncode == ExitCodes.CALL_ERROR:
            print(
                f"{bcolors.FAIL}Compilation of {file} called with wrong number of arguments{bcolors.ENDC}"
            )
        elif res.returncode == ExitCodes.FILE_NOT_FOUND_ERROR:
            print(f"{bcolors.FAIL}{file} not found{bcolors.ENDC}")
        elif res.returncode == ExitCodes.SCANNING_ERROR:
            print(f"{bcolors.FAIL}Error during scanning phase of {file}{bcolors.ENDC}")
        elif res.returncode == ExitCodes.PARSING_ERROR:
            print(f"{bcolors.FAIL}Error during parsing phase of {file}{bcolors.ENDC}")
        elif res.returncode == ExitCodes.CODE_GEN_ERROR:
            print(f"{bcolors.FAIL}Error during code generation of {file}{bcolors.ENDC}")
        if args.print_stderr:
            print(f"{res.stderr.decode()}")
        passing = False
    elif compilation_should_fail and res.returncode == 0:
        print(
            f"{bcolors.FAIL}Cobalt should have flagged errros with {file}{bcolors.ENDC}"
        )

    if not compilation_should_fail and passing:
        cobalt_res = subprocess.run(
            f"gcc {assembly_file} -o cobalt_exec.out",
            cwd=cur_dir,
            shell=True,
            capture_output=True,
        )

        if res.returncode != 0:
            print(f"{bcolors.FAIL}Cobalt generated invalid assembly{bcolors.ENDC}")
            passing = False

    if not compilation_should_fail and passing:
        assembly_files.append(assembly_file)

        cobalt_res = subprocess.run(
            ["./cobalt_exec.out"],
            cwd=cur_dir,
            shell=True,
            capture_output=True,
        )

        gcc_res = subprocess.run(
            ["./gcc_exec.out"],
            cwd=cur_dir,
            shell=True,
            capture_output=True,
        )

        if gcc_res.returncode != cobalt_res.returncode:
            print(
                f"{bcolors.FAIL}Expected return code {gcc_res.returncode}, got {cobalt_res.returncode} instead{bcolors.ENDC}"
            )
            passing = False

    if passing:
        print(f"{bcolors.OKGREEN}{file} passed tests{bcolors.ENDC}")
        correct += 1

subprocess.run(
    "rm *.out",
    cwd=cur_dir,
    shell=True,
    capture_output=True,
)

for assembly_file in assembly_files:
    os.remove(assembly_file)

if total > 0 and correct == total:
    print(f"{bcolors.OKBLUE}All tests passed ({correct}/{total}){bcolors.ENDC}")
    exit(0)


if total == 0:
    print(
        f"{bcolors.FAIL}No test has been run. Considering pipeline as failing.{bcolors.ENDC}"
    )
else:
    print(f"{bcolors.FAIL}Some tests failed. ({correct}/{total}){bcolors.ENDC}")
exit(1)
