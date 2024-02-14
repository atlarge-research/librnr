import os
from pathlib import Path


def a(file_path):
    path = Path(file_path)
    try:
        path.write_text(path.read_text(encoding="utf16"), encoding="utf8")
    except:
        return

def traverse_and_execute(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".log"):
                file_path = os.path.join(root, file)
                a(file_path)

# Specify the root directory from where you want to start the traversal
root_directory = "."

# Call the function to traverse and execute
traverse_and_execute(root_directory)
