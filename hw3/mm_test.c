#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

/* Function pointers to hw3 functions */
void* (*mm_malloc)(size_t);
void* (*mm_realloc)(void*, size_t);
void (*mm_free)(void*);

void (*print_list)();

void load_alloc_functions() {
    void *handle = dlopen("hw3lib.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    char* error;
    mm_malloc = dlsym(handle, "mm_malloc");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    mm_realloc = dlsym(handle, "mm_realloc");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    mm_free = dlsym(handle, "mm_free");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    print_list = dlsym(handle, "print_list");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
}

int main() {
    load_alloc_functions();

    // print_list();
    // int *data = (int*) mm_malloc(sizeof(int));
    // int *data2 = (int*) mm_malloc(sizeof(int) * 3);
    // int *data3 = (int*) mm_malloc(sizeof(int));
    // assert(data != NULL);
    // printf("sizeof(int) = %lu\n", sizeof(int));
    // printf("sizeof(char) = %lu\n", sizeof(char));
    // printf("sizeof(short) = %lu\n", sizeof(short));
    // print_list();
    // data[0] = 0x162;
    // data2[0] = 0x6e;
    // data2[1] = 0x65;
    // data2[2] = 0x42;

    /* Shows writes can bleed into other sections. */
    // data2[3] = 0x78;
    // data2[4] = 0x78;
    // data2[9] = 0x78;
    // mm_free(data2);
    // print_list();


    // print_list();
    // data = mm_realloc(data, sizeof(int) * 2);
    // print_list();
    // data = mm_realloc(data, 0);
    // print_list();
    // data = mm_realloc(NULL, sizeof(int) * 3);
    // print_list();
    // data = mm_realloc(data, sizeof(int) * 1);
    // print_list();


    int *data4 = (int*) mm_malloc(sizeof(int) * 10);
    data4[0] = 0x6e;
    print_list();
    int *data5 = (int*) mm_malloc(sizeof(int) * 10);
    data5[0] = 0x65;
    print_list();
    // int *data4 = (int*) mm_malloc(sizeof(int) * 100000000000000000000000);
    data4 = mm_realloc(data4, sizeof(int) * 20);
    print_list();
    int *data6 = (int*) mm_malloc(sizeof(int) * 10);
    data6[0] = 0x42;
    print_list();
    int *data7 = mm_realloc(data4, sizeof(int) * 100000000000000000000000);

    print_list();

    // mm_free(data);
    // mm_free(data2);
    // mm_free(data3);
    mm_free(data4);
    print_list();
    mm_free(data5);
    print_list();
    mm_free(data6);
    print_list();
    mm_free(data7);
    print_list();
    printf("malloc test successful!\n");
    return 0;
}
