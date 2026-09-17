#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  // TODO
  struct naive_data *data = malloc(sizeof(struct naive_data));
  if (data == NULL) {
    fprintf(stderr, "Failed to allocate memory for naive_data\n");
    return NULL;
  }
  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data* data) {
  // TODO
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, int64_t needle) {
  // TODO
  if (data == NULL || data->n == 0) {
    return NULL;
  }

  for (int i = 0; i < data->n; i++) {
    if (data->rs[i].osm_id == needle) {
      return &data->rs[i];
    }
  }

  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_naive,
                    (free_index_fn)free_naive,
                    (lookup_fn)lookup_naive);
}
