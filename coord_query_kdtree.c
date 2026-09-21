#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include "record.h"
#include "coord_query.h"


static struct kdtree(struct record **points, int n, int depth) {
    if (n <= 0) {
      return NULL;
    }

    // 2 dimensions, 0 = longitude and 1 = latitude.
    int axis = depth % 2;
    int mid = n / 2;

    // Select median by axis from points
    select(points, 0, n, mid, axis);

    struct kdtree_node *node = malloc(sizeof(struct kdtree_node));
    if (node == NULL) {
      fprintf(stderr, "Failed to allocate memory for naive_data\n")
    }

    node->point = points[median];
    node->axis = axis;
    
    // Points before median
    node->left = kdtree(points, mid, depth + 1);

    // Points after median
    node->right = kdtree(points + mid + 1, n - mid - 1, depth + 1);

    return node;
}

static void free_node(struct kdtree_node *node) {
  if (node == NULL) {
    return;
  }
  
  free_node(node->left);
  free_node(node->right);
  free(node);
}