import os
from pathlib import Path
import sys


def create_h_for_c_file(cpp_path):
    hpp_path = cpp_path.with_suffix(".hpp")
    if not hpp_path.exists():
        hpp_path.touch()


def create_hpp_for_cpp_file(cpp_path):
    hpp_path = cpp_path.with_suffix(".hpp")
    if not hpp_path.exists():
        hpp_path.touch()


def process_directory(source_dir):
    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith((".cpp", ".cxx")):
                cpp_path = Path(root) / file
                create_hpp_for_cpp_file(cpp_path)
            if file.endswith((".c",)):
                c_path = Path(root) / file
                create_h_for_c_file(c_path)


if __name__ == "__main__":
    source_directory = Path(sys.argv[1])
    process_directory(source_directory)
