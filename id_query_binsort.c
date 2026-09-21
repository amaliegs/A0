#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
    int64_t osm_id;
    const struct record *record;
};

struct binsort_data {
    struct index_record *irs;
    int n;
};

static int compare_index_records(const void *a, const void *b) {
    const struct index_record *ia = a;
    const struct index_record *ib = b;

    if (ia->osm_id < ib->osm_id) return -1;
    if (ia->osm_id > ib->osm_id) return 1;
    return 0;
}

struct binsort_data* mk_binsort(struct record* rs, int n) {
    struct binsort_data *data = malloc(sizeof(struct binsort_data)); 
    if (data == NULL) {
        fprintf(stderr, "Failed to allocate memory for binsort_data\n");
        return NULL;
    }

    data->irs = malloc(n * sizeof(struct index_record));
    if (data->irs == NULL) {
        fprintf(stderr, "Failed to allocaye memory for index_record array\n");
        free(data);
        return NULL;
    }
    data->n = n;
    
    for (int i = 0; i < n; i++) {
        data->irs[i].osm_id = rs[i].osm_id;
        data->irs[i].record = &rs[i];
    }
    
    qsort(data->irs, n, sizeof(struct index_record), compare_index_records);

    return data;
}

void free_binsort(struct binsort_data* data) {
    free(data->irs);
    free(data);
}

const struct record* lookup_binsort(struct binsort_data *data, int64_t needle) {
    if (data == NULL || data ->n == 0) {
    return NULL;
    }

    int lo = 0; 
    int hi = data->n - 1;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int64_t mid_id = data->irs[mid].osm_id;

        if (mid_id == needle) {
            return data->irs[mid].record;
        } else if (mid_id < needle) {
            lo = mid + 1; 
        } else {
            hi = mid -1; 
        }
    }

    return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binsort,
                    (free_index_fn)free_binsort,
                    (lookup_fn)lookup_binsort);
}
