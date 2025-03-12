# Parallel Inverted Index Using Map-Reduce - Implementation

## Mappers

Mappers handle the initial processing of input files, each mapper processing one file at a time. For each word in a file, mappers will:
- remove non alphabetical charaters and convert all letters to lowercase
- add the word to a partial list of unique words

### Data structures

-  **Partial Lists**
    - stored in a vector of `num_files` elements
    - each element corresponds to a file's partial list, divided into 26 sublists based on the first letter of each word
    - sublists are implemented as unordered sets, to ensure uniqueness of words

### Dynamic work distribution

Mappers use a shared `files_processed` counter:
- each mapper increments the counter atomically to claim a file for processing
- this continues until all files are processed, ensuring dynamic and balanced workload distribution


## Reducers

Reducers aggregate and sort the words processed by mappers, each reducer processing one letter at a time. For each letter, reducers will:
- combine all occurrences of words starting with that letter from the partial lists, into an aggregated list
- sort the aggregated list, based on
    - the number of files containing the word (descending order)
    - alphabetically
- write the sorted aggregated lists to the output file

### Data structures
- **Aggregated Lists**
    - initially stored in an unordered map (key = word, value = file IDs)
    - converted to a vector of (word, file IDs) pairs for sorting

### Dynamic work distribution
Similarly to mappers, reducers use a shared `letters_processed` counter to claim a letter to process. This counter stops at 26 (the number of letters in the alphabet).


## Threads
To minimise resource usage, the program reuses threads between the mapper and reducer phases. 

### Role assignment
- only `max(num_mappers, num_reducers)` threads are created
- threads decrement a shared `mappers_available` counter to assume a mapper role
- after all mappers finish their tasks, threads are reassigned as reducers, using a `reducers_available` counter

### Synchronization
- _Atomic counters_ - ensure safe updates to shared counters
- _Memory independence_ - mappers and reducers read and write in separate memory locations, avoiding the need for complex synchronization methods
- _Barrier_ - threads wait at a barrier until all mappers finish their execution, before proceeding with the reducer phase 


## Scalability
The program adapts to any given number of threads, however:
- the number of active mappers is capped at the number of files to be processed
- the number of active reducers is capped at 26 (the number of letters in the alphabet)
- any additional mappers or reducers will remain idle
<br>

Additionally, work imbalance can occur if all words fall under a small subset of letters, but this is rare in real-world scenarios.

### Potential Enhancements
#### More granular work distribution:
- split reducer tasks further (e.g. process subsets of words or divide partial lists across reducers)
- could improve scalability on systems with more then 26 threads, but may introduce overhead on smaller systems

#### Distributed systems:
- Operations are performed on separate memory locations, making the implementation suitable for distributed systems, with minimal communication overhead

### Trade-offs
Given the focus on systems with 4-16 threads (common in testing and home systems), the current implementation balances scalability and simplicity, as more granular work splitting would reduce efficiency for typical scenarios.