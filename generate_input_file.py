import os


# run this script from the root of the repository


# modify this with target directory
target_directory = "test/test_in"

output_file = "input.txt"

with open(output_file, "w") as outfile:
    all_files = [
        os.path.relpath(os.path.join(root, file), ".") 
        for root, _, files in os.walk(target_directory) 
        for file in files
    ]
    outfile.write("\n".join(all_files))

print(f"File paths written to {output_file}")
