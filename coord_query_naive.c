#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"

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

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  // TODO
  if (data == NULL || data->n == 0) {
    return NULL;
  }

  const struct record *best_record = NULL;
  double min_dist_sq = -1.0;
  
  for (int i = 0; i < data->n; i++) {
    double d_lon = data->rs[i].lon - lon;
    double d_lat = data->rs[i].lat - lat;

    // Euclidean distance squared
    double dist_sq = d_lon * d_lon + d_lat * d_lat;

    if (best_record == NULL || dist_sq < min_dist_sq) {
      min_dist_sq = dist_sq;
      best_record = &data->rs[i];
    }
  }

  return best_record;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
