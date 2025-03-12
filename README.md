# Parallel Inverted Index Using Map-Reduce

Parallel computation of an inverted index using the map-reduce paradigm.

## Usage
Builing the program: 
        
    make

Clean up: 

    make clean

Running:

    ./inverted-index num_mappers num_reducers input_file

- `num_mappers` - number of mapper threads <br>
- `num_reducers` - number of reducer threads <br>
- `input_file` - a text file containing the paths of the files to be processed, one path per line
<br>

Mapper threads are reused as reducers, so the total number of threads used is `max(num_mappers, num_reducers)`.

## Output
The program generates an `out` folder, containing files `a.txt` to `z.txt`.<br>
Each file contains:
- the words starting with the corresponding letter
- the IDs of the files where the word appears
<br>

Words are sorted by
1. the number of files they appear in (descending order)
2. alphabetically


## Other files
- `generate_input_file.py` - helper script to generate input files for testing <br>
- `test` folder - contains sample test files

## Performance
The program demonstrates a 3x speedup on an 8-thread system when compared to a single-threaded run

<br>

See [IMPLEMENTATION.md](IMPLEMENTATION.md) for detailed information about the program's architecture and implementation.

***

Developed as part of the Parallel and Distributed Algorithms course at UNSTPB.