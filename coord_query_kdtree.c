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
struct Node* build_kdtree(Point points[], int n, int depth) {
    if (n <= 0) return NULL;

    int axis = depth % k;
  // TODO
  struct kdtree_data *data = malloc(sizeof(struct kdtree_data));
  if (n <= 0) return NULL {
    fprintf(stderr, "Failed to allocate memory for kdtree_data\n");
    return NULL;
  }
  data->rs = rs;
  data->n = n;
  return data;
}

node create_node(points, depth) {
    axis <- depth mod d
    select median by axis from points
    node <- new node
    node.point <- median
    node.axis <- axis
    node.left <- create_node (points before median, depth+1)
    node.right <- create_node (points after median, depth+1)
    return node
}

void free_kdtree(struct kdtree_data* data) {
  // TODO
  free(data);
}
const struct record* lookup_kdtree(struct kdtree_data *data, double lon, double lat) {
    if (Node == NULL) {
        return NULL;
    } else if { 
        (Node.points >
  
}
int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
