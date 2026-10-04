#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 5
#define MAX_NODES 50
#define NAME_LEN 32

typedef struct Node {
    char name[NAME_LEN];
    struct Node *children[MAX_CHILDREN];
    int child_count;
} Node;

static Node *create_node(const char *name) {
    Node *node = malloc(sizeof *node);
    if (node == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    snprintf(node->name, sizeof node->name, "%s", name);
    node->child_count = 0;
    return node;
}

static Node *add_child(Node *parent, const char *name) {
    if (parent->child_count >= MAX_CHILDREN) {
        fprintf(stderr, "Error: maximum number of children exceeded.\n");
        exit(EXIT_FAILURE);
    }
    Node *child = create_node(name);
    parent->children[parent->child_count++] = child;
    return child;
}

static Node *build_hierarchy(void) {
    Node *ceo = create_node("CEO");
    add_child(ceo, "HR");
    add_child(ceo, "Finance");
    Node *it = add_child(ceo, "IT");
    Node *development = add_child(it, "Development");
    add_child(it, "Testing");
    add_child(development, "Frontend");
    add_child(development, "Backend");
    return ceo;
}

static int level_order(Node *root, char order[][NAME_LEN]) {
    Node *queue[MAX_NODES];
    int front = 0, rear = 0, total = 0, level = 0;
    queue[rear++] = root;

    while (front < rear) {
        int nodes_this_level = rear - front;
        printf("Level %d: ", level++);
        for (int i = 0; i < nodes_this_level; ++i) {
            Node *current = queue[front++];
            printf("%s%s", current->name, i + 1 < nodes_this_level ? ", " : "\n");
            snprintf(order[total++], NAME_LEN, "%s", current->name);
            for (int j = 0; j < current->child_count; ++j) {
                if (rear >= MAX_NODES) {
                    fprintf(stderr, "Error: queue capacity exceeded.\n");
                    exit(EXIT_FAILURE);
                }
                queue[rear++] = current->children[j];
            }
        }
    }
    return total;
}

/* Height is measured in edges; a leaf has height zero. */
static int tree_height(const Node *node) {
    int largest_child_height = -1;
    for (int i = 0; i < node->child_count; ++i) {
        int child_height = tree_height(node->children[i]);
        if (child_height > largest_child_height)
            largest_child_height = child_height;
    }
    return largest_child_height + 1;
}

static int count_nodes(const Node *node) {
    int total = 1;
    for (int i = 0; i < node->child_count; ++i)
        total += count_nodes(node->children[i]);
    return total;
}

static void free_tree(Node *node) {
    for (int i = 0; i < node->child_count; ++i)
        free_tree(node->children[i]);
    free(node);
}

static int linear_search(char names[][NAME_LEN], int count, const char *key, int *comparisons) {
    *comparisons = 0;
    for (int i = 0; i < count; ++i) {
        ++*comparisons;
        if (strcmp(names[i], key) == 0)
            return i;
    }
    return -1;
}

/* Binary search requires the input array to be alphabetically sorted. */
static int binary_search(char names[][NAME_LEN], int count, const char *key, int *comparisons) {
    int low = 0, high = count - 1;
    *comparisons = 0;
    while (low <= high) {
        int middle = low + (high - low) / 2;
        ++*comparisons;
        int result = strcmp(names[middle], key);
        if (result == 0)
            return middle;
        if (result < 0)
            low = middle + 1;
        else
            high = middle - 1;
    }
    return -1;
}

static void sort_names(char names[][NAME_LEN], int count) {
    for (int i = 1; i < count; ++i) {
        char current[NAME_LEN];
        snprintf(current, sizeof current, "%s", names[i]);
        int j = i - 1;
        while (j >= 0 && strcmp(names[j], current) > 0) {
            snprintf(names[j + 1], NAME_LEN, "%s", names[j]);
            --j;
        }
        snprintf(names[j + 1], NAME_LEN, "%s", current);
    }
}

int main(void) {
    Node *root = build_hierarchy();
    char traversal[MAX_NODES][NAME_LEN];
    char sorted[MAX_NODES][NAME_LEN];

    puts("=== a) Level-order traversal ===");
    int count = level_order(root, traversal);
    printf("Traversal order: ");
    for (int i = 0; i < count; ++i)
        printf("%s%s", traversal[i], i + 1 < count ? " -> " : "\n");

    puts("\n=== b) Linear vs binary search ===");
    for (int i = 0; i < count; ++i)
        snprintf(sorted[i], NAME_LEN, "%s", traversal[i]);
    sort_names(sorted, count);
    printf("Sorted list: ");
    for (int i = 0; i < count; ++i)
        printf("%s%s", sorted[i], i + 1 < count ? ", " : "\n\n");

    const char *keys[] = {"CEO", "Finance", "Backend", "Testing", "Sales"};
    const int search_count = (int)(sizeof keys / sizeof keys[0]);
    int linear_total = 0, binary_total = 0;
    printf("%-12s%8s%8s   %s\n", "Department", "Linear", "Binary", "Found");
    for (int i = 0; i < search_count; ++i) {
        int linear_comparisons, binary_comparisons;
        int linear_result = linear_search(traversal, count, keys[i], &linear_comparisons);
        int binary_result = binary_search(sorted, count, keys[i], &binary_comparisons);
        linear_total += linear_comparisons;
        binary_total += binary_comparisons;
        printf("%-12s%8d%8d   %s\n", keys[i], linear_comparisons,
               binary_comparisons, linear_result != -1 && binary_result != -1 ? "Yes" : "No");
    }
    printf("%-12s%8.1f%8.1f\n", "Average",
           (double)linear_total / search_count,
           (double)binary_total / search_count);

    puts("\n=== c) Analysis ===");
    int height = tree_height(root);
    printf("Number of nodes : %d\n", count_nodes(root));
    printf("Tree height     : %d edges | %d levels\n", height, height + 1);
    puts("Traversal       : Level-order visits each node once, O(n) time.");
    puts("Search          : Linear O(n); binary O(log n) after sorting.");
    puts("Sorting         : Insertion sort O(n^2) for this implementation.");
    puts("Use             : A general tree models reporting relationships; a sorted list supports department lookup.");

    free_tree(root);
    return EXIT_SUCCESS;
}
