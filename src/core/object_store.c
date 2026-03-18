#include "../include/minigit.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static ObjectStore *global_store = NULL;

ObjectStore* init_object_store(void) {
    if (global_store) {
        return global_store;
    }
    
    global_store = (ObjectStore*)malloc(sizeof(ObjectStore));
    if (!global_store) {
        return NULL;
    }
    
    for (int i = 0; i < 256; i++) {
        global_store->buckets[i] = NULL;
        global_store->bucket_count[i] = 0;
    }
    global_store->total_count = 0;
    
    return global_store;
}

static int get_bucket_index(const unsigned char *hash) {
    return hash[0];
}

void add_object(ObjectStore *store, void *obj, const unsigned char *hash) {
    if (!store || !obj || !hash) return;
    
    int bucket_idx = get_bucket_index(hash);
    
    // Проверяем, нет ли уже такого
    HashBucket *current = store->buckets[bucket_idx];
    while (current) {
        if (memcmp(current->hash, hash, SHA1_HASH_SIZE) == 0) {
            return;  // Уже есть
        }
        current = current->next;
    }
    
    // Создаем новый бакет
    HashBucket *new_bucket = (HashBucket*)malloc(sizeof(HashBucket));
    if (!new_bucket) return;
    
    new_bucket->object = obj;
    memcpy(new_bucket->hash, hash, SHA1_HASH_SIZE);
    new_bucket->next = store->buckets[bucket_idx];
    store->buckets[bucket_idx] = new_bucket;
    store->bucket_count[bucket_idx]++;
    store->total_count++;
}

void* get_object(ObjectStore *store, const unsigned char *hash) {
    if (!store || !hash) return NULL;
    
    int bucket_idx = get_bucket_index(hash);
    HashBucket *current = store->buckets[bucket_idx];
    
    while (current) {
        if (memcmp(current->hash, hash, SHA1_HASH_SIZE) == 0) {
            return current->object;
        }
        current = current->next;
    }
    
    return NULL;
}

int has_object(ObjectStore *store, const unsigned char *hash) {
    return get_object(store, hash) != NULL;
}

ObjectStore* get_global_store(void) {
    if (!global_store) {
        return init_object_store();
    }
    return global_store;
}

void print_store_stats(ObjectStore *store) {
    if (!store) return;
    
    int used_buckets = 0;
    int max_chain = 0;
    
    for (int i = 0; i < 256; i++) {
        if (store->bucket_count[i] > 0) {
            used_buckets++;
            if (store->bucket_count[i] > max_chain) {
                max_chain = store->bucket_count[i];
            }
        }
    }
    
    // Статистика не выводится, только собирается
}

void free_object_store(ObjectStore *store) {
    if (!store) return;
    
    for (int i = 0; i < 256; i++) {
        HashBucket *current = store->buckets[i];
        while (current) {
            HashBucket *next = current->next;
            free(current);
            current = next;
        }
    }
    
    free(store);
    if (store == global_store) global_store = NULL;
}