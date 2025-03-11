#ifndef MAP_REDUCE_H_
#define MAP_REDUCE_H_

#include <vector>
#include <string>
#include <list>
#include <unordered_set>
#include <atomic>
#include <algorithm>
#include <pthread.h>

#define NUM_LETTERS 26

typedef std::vector<std::unordered_set<std::string>>        PartialList;
typedef std::pair<std::string, std::vector<unsigned int>>   AggregatedList;

struct ThreadArg {
    std::atomic<int>            *remaining_mappers;
    std::atomic<int>            *remaining_reducers;
    pthread_barrier_t           *mappers_done_barrier;
    std::vector<std::string>    *files;
    std::atomic<unsigned int>   *files_processed;
    std::vector<PartialList>    *partial_lists;
    std::atomic<unsigned int>   *letters_processed;
};

void mapper_loop(ThreadArg *args);
void reducer_loop(ThreadArg *args);
void *thread_function(void *arg);

#endif