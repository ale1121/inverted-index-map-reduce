#include <fstream>
#include <cctype>
#include <unordered_map>
#include "map_reduce.h"

using namespace std;


void *thread_function(void *arg) {
    ThreadArg *thread_args = static_cast<ThreadArg *>(arg);
    
    // take on the role of mapper if still available
    int rem_mappers = thread_args->remaining_mappers->fetch_sub(1);
    if (rem_mappers > 0) {
        mapper_loop(thread_args);
    }

    // wait until all mappers finish their tasks
    pthread_barrier_wait(thread_args->mappers_done_barrier);

    // take on the role of reducer if still available
    int rem_reducers = thread_args->remaining_reducers->fetch_sub(1);
    if (rem_reducers > 0) {
        reducer_loop(thread_args);
    }

    return NULL;
}


void mapper_loop(ThreadArg *args) {
    vector<string> *files = args->files;
    atomic<unsigned int> *files_processed = args->files_processed;
    vector<PartialList> *partial_lists = args->partial_lists;

    while (true) {
        // get the next available file
        unsigned int file_idx = files_processed->fetch_add(1);
        if (file_idx >= files->size()) {
            // all files have been processed
            break;
        }

        string file = (*files)[file_idx];
        ifstream fin(file);
        if (!fin.is_open()) {
            fprintf(stderr, "failed to open file %s\n", file.c_str());
            // move on to the next file
            continue;
        }

        string word;

        while (fin >> word) {
            string new_word;
            new_word.reserve(word.size());

            // remove non alphabetic characters and
            // convert all letters to lowercase
            for (char &c : word) {
                if (isalpha(c)) {
                    new_word.push_back(tolower(c));
                }
            }
            
            if (new_word == "") {
                continue;
            }

            // add word to partial list
            unsigned int letter_idx = new_word[0] - 'a';
            (*partial_lists)[file_idx][letter_idx].insert(new_word);
        }

        fin.close();
    }
}


// sort the vector of aggregated lists, first by number of file_ids associated,
// then alphabetically by word
void sort_list(vector<AggregatedList> &v) {
    sort(v.begin(), v.end(),
            [](const AggregatedList &al1, const AggregatedList &al2){

        int vec_len_diff = static_cast<int>(al1.second.size()) - 
                            static_cast<int>(al2.second.size());
        
        if (vec_len_diff != 0) {
            return vec_len_diff > 0;
        } else {
            return al1.first < al2.first;
        }
    });
}


// write the sorted aggregates lists for the given letter in letter.txt
void write_to_file(
        char letter, vector<AggregatedList> &aggregated_lists) {

    char out_file[10];
    sprintf(out_file, "out/%c.txt", letter);
    ofstream fout(out_file);
    if (!fout.is_open()) {
        fprintf(stderr, "failed to open file %s\n", out_file);
        return;
    }

    for (auto &[word, files] : aggregated_lists) {
        fout << word << ": [";
        for (unsigned int i = 0; i < files.size() - 1; i++) {
            fout << files[i] << " ";
        }
        fout << files[files.size() - 1] << "]" << endl;
    }
}


void reducer_loop(ThreadArg *args) {
    vector<PartialList> *partial_lists = args->partial_lists;
    atomic<unsigned int> *letters_processed = args->letters_processed;

    while (true) {
        // get the next available letter
        unsigned int letter_idx = letters_processed->fetch_add(1);
        if (letter_idx >= 26) {
            // all letters have been processed
            break;
        }

        char letter = letter_idx + 'a';
        unordered_map<string, vector<unsigned int>> aggregated_file_ids;

        // combine all partial lists for words starting with letter 
        for (unsigned int file_id = 0; file_id < partial_lists->size(); file_id++) {
            for (auto &word : (*partial_lists)[file_id][letter_idx]) {
                // file_id + 1 because the files should be indexed from 1
                aggregated_file_ids[word].push_back(file_id + 1);
            }
        }

        // sort aggregated lists
        vector<AggregatedList> aggregated_lists(
                aggregated_file_ids.begin(), aggregated_file_ids.end());
        sort_list(aggregated_lists);

        write_to_file(letter, aggregated_lists);
    }
}
