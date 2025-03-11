#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <atomic>
#include <sys/stat.h>
#include "map_reduce.h"

using namespace std;


int main(int argc, char **argv)
{
    if(argc != 4) {
        fprintf(stderr, 
                "usage: ./inverted-index <num_mappers> <num_reducers> <input_file>\n");
        exit(-1);
    }

    // process arguments
    unsigned int num_mappers, num_reducers;
    try {
        num_mappers = static_cast<unsigned int>(stoi(argv[1]));
        num_reducers = static_cast<unsigned int>(stoi(argv[2]));
    } catch (exception &e) {
        fprintf(stderr, 
                "usage: ./tema1 <num_mappers> <num_reducers> <input_file>\n"
                "num_mappers and num_reducers should be integers\n");
        exit(-1);
    }
    string input_file = argv[3];

    // open input file
    ifstream fin(input_file);
    if (!fin.is_open()) {
        fprintf(stderr, "could not open input file %s\n", input_file.c_str());
        exit(-1);
    }

    // read input file
    vector<string> files;
    string file;
    while (getline(fin, file)) {
        files.push_back(file);
    }
    fin.close();
    unsigned int num_files = files.size();

    // create out directory
    const char *out_dir = "out";
    mkdir(out_dir, 0777);

    // initialise thread argument elements
    vector<PartialList> partial_lists(num_files, PartialList(NUM_LETTERS));
    atomic<unsigned int> files_processed = 0;
    atomic<unsigned int> letters_processed = 0;
    atomic<int> remaining_mappers = num_mappers;
    atomic<int> remaining_reducers = num_reducers;

    // by reusing mapper threads as reducers, reduce the total
    // number of threads to max(num_mappers, num_reducers)
    unsigned int num_threads = max(num_mappers, num_reducers);

    // initialise barrier
    pthread_barrier_t mappers_done_barrier;
    int ret = pthread_barrier_init(&mappers_done_barrier, NULL, num_threads);
    if (ret != 0) {
        fprintf(stderr, "error initialising barrier\n");
        exit(-1);
    }

    // create thread argument
    ThreadArg *arg = new ThreadArg;
    arg->remaining_mappers = &remaining_mappers;
    arg->remaining_reducers = &remaining_reducers;
    arg->mappers_done_barrier = &mappers_done_barrier;
    arg->files = &files;
    arg->files_processed = &files_processed;
    arg->partial_lists = &partial_lists;
    arg->letters_processed = &letters_processed;

    // create threads
    vector<pthread_t> threads(num_threads);
    for (unsigned int i = 0; i < num_threads; i++) {
        int ret = pthread_create(&threads[i], NULL, thread_function, static_cast<void *>(arg));
        if (ret != 0) {
            fprintf(stderr, "error while creating thread %u\n", i);
            exit(-1);
        }
    }

    // join threads
    for (unsigned int i = 0; i < num_threads; i++) {
        int ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            fprintf(stderr, "error while joining thread %u\n", i);
            exit(-1);
        }
    }

    // free memory
    pthread_barrier_destroy(&mappers_done_barrier);
    delete(arg);

    return 0;
}