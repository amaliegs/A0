#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"


// One node of the tree
struct kdtree_node {

    // A pointer to the "struct record" defined in record.h.
    const struct record *point;
    
    // axis = 0 denote longitude and axis = 1 denote latitude.
    int axis;

    // Pointers to the node's two children.
    struct kdtree_node *left;
    struct kdtree_node *right;
};

// The full kdtree index:.
struct kdtree_data {

    // Pointer to the original full array of records.
    struct record *rs;

    // How many records are in rs.
    int n;

    // Pointer to the root node of the tree.
    struct kdtree_node *root;
};

// Returns record r's coordinate along the given axis.
static double record_axis(const struct record *r, int axis) {

    // axis = 0 denote longitude and axis = 1 denote latitude.
    if (axis == 0) {
        return r->lon;
    } else {
        return r->lat;
    }
}

// Returns the squared euclidian distance between record r and (lon, lat).
// Comparing squared distances gives the same ordering as comparing 
// euclidean distance, so there is no need to calculate sqrt()
static double dist_sq(const struct record *r, double lon, double lat) {
    return (r->lon - lon) * (r->lon - lon) + (r->lat - lat) * (r->lat - lat);
}

// Merges the two sorted halves [low, mid) and (mid, high] into temp, 
// then copies the sorted result back into points.
static void merge(struct record **points, struct record **temp, int low, int mid, int high, int axis) {
    int i = low;
    int j = mid;
    int k = low;

    while (i < mid && j < high) {
        if (record_axis(points[i], axis) <=record_axis(points[j], axis)) {
            temp[k++] = points[i++];
        } else {
            temp[k++] = points[j++];
        }
    }

    while (i < mid) {
        temp[k++] = points[i++];
    }

    while (j < high) {
        temp[k++] = points[j++];
    }

    for (i = low; i < high; i++) {
        points[i] = temp[i];
    }
}

// Using mergesort to recursively split points into two halves, sorts each half,
// and merges them back together based on the given coordinate axis.
static void mergesort(struct record **points, struct record **temp, int low, int high, int axis) {
    if (high - low <= 1) {
        return;
    }

    int mid = low + (high - low) / 2;

    mergesort(points, temp, low, mid, axis);
    mergesort(points, temp, mid, high, axis);

    merge(points, temp, low, mid, high, axis);
}

// Building the actual tree.
static struct kdtree_node* kdtree(struct record **points, int n, int depth) {
    if (n <= 0) {
      return NULL;
    }

    // 2 dimensions, 0 = longitude and 1 = latitude.
    int axis = depth % 2;
    int mid = n / 2;

    // Temporary array used by mergesort
    struct record **temp = malloc(sizeof(struct record *) * n);
    if (temp == NULL) {
        fprintf(stderr, "Failed to allocate memory for mergesort\n");
        return NULL;
    }

    // Sort points by the current axis.
    mergesort(points, temp, 0, n, axis);

    // Free temporary allocated memory
    free(temp);

    // Allocate memory for a new node in the kdtree.
    struct kdtree_node *node = malloc(sizeof(struct kdtree_node));
    if (node == NULL) {
      fprintf(stderr, "Failed to allocate memory for naive_data\n");
      return NULL;
    }

    // Set the node's point to the median and store the splitting axis.
    node->point = points[mid];
    node->axis = axis;
    
    // Points before median
    node->left = kdtree(points, mid, depth + 1);

    // Points after median
    node->right = kdtree(points + mid + 1, n - mid - 1, depth + 1);

    return node;
}

// Recursively frees all nodes in the kdtree.
static void free_node(struct kdtree_node *node) {
  if (node == NULL) {
    return;
  }
  
  free_node(node->left);
  free_node(node->right);
  free(node);
}

// Creates a kdtree from the given array of records.
struct kdtree_data* mk_kdtree(struct record* rs, int n) {
    
    // Allocate memory for the kdtree data structure. 
    struct kdtree_data *data = malloc(sizeof(struct kdtree_data));
    if (data == NULL) {
        fprintf(stderr, "Failed to allocate memory for kdtree_data\n");
        return NULL;
    }

    // Store the input records and initialize the tree root
    data->rs = rs;
    data->n = n;
    data->root = NULL;

    if (n == 0) {
        return data;
    }

    // Allocate an array of pointers to the records.
    struct record **points = malloc(sizeof(struct record*) * n);
    if (points == NULL) {
        fprintf(stderr, "Failed to allocate memory for points array\n");
        free(data);
        return NULL;
    }

    // Initialize each pointer to the corresponding record.
    for (int i = 0; i < n; i++) {
        points[i] = &rs[i];
    }

    // Build the kdtree from the record pointers.
    data->root = kdtree(points, n, 0);

    free(points);
    return data;
}

// Frees the entire kdtree and its associated data.
void free_kdtree(struct kdtree_data* data) {
    if (data == NULL) {
        return;
    }

    free_node(data->root);
    free(data);
}

// Represents a query point with longitude and latitude.
struct query_point {
    double lon;
    double lat;
};

// Stores the closest record found and its squared distance to the query point.
struct closest_result {
    const struct record *record;
    double dist_sq;
};

// Returns the query point's coordinate along the given axis.
static double query_axis(struct query_point query, int axis) {
    if (axis == 0) {
        return query.lon;
    } else {
        return query.lat;
    }
}

// Recursively searches the kdtree for the record closest to the query point
static void lookup(struct closest_result *closest, struct query_point query, const struct kdtree_node *node) {
    if (node == NULL) {
        return;
    }

    // Calculate the squared distance from the current node to the query point.
    double d = dist_sq(node->point, query.lon, query.lat);

    // Update the closest record if a node is closer
    if (closest->record == NULL || d < closest->dist_sq) {
        closest->record = node->point;
        closest->dist_sq = d;
    }

    // Calculate the difference between the node and query along the current axis.
    double diff = record_axis(node->point, node->axis) - query_axis(query, node->axis);

    // Search the relevant subtrees. The other subtree is searched if it
    // could still contain a point closer than the current best match.
    double diff_sq = diff * diff;

    if (diff >= 0 || closest->dist_sq > diff_sq) {
        lookup(closest, query, node->left);
    }

    if (diff <= 0 || closest->dist_sq > diff_sq) {
        lookup(closest, query, node->right);
    }
}

// Finds and returns the record closest to the given longitude and latitude.
const struct record* lookup_kdtree(struct kdtree_data *data, double lon, double lat) {
    if (data == NULL || data ->root == NULL) {
        return NULL;
    }
    
    struct query_point query = {
        .lon = lon, .lat = lat
    };
    struct closest_result closest = {
        .record = NULL, .dist_sq = 0.0
    };

    lookup(&closest, query, data->root);
    return closest.record;
}

int main(int argc, char** argv) {
    return coord_query_loop(argc, argv,
                            (mk_index_fn)mk_kdtree,
                            (free_index_fn)free_kdtree,
                            (lookup_fn)lookup_kdtree);
}