#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#include <stdint.h>

#define FNV_PRIME 16777619L
#define FNV_OFFSET 2166136261L

#define HTABLE_MAX 0.75

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define ALLOCATE(type, capacity) (type*)realloc(NULL, capacity)
#define FREE(type, pointer, capacity) free(pointer)

void init_hashtable(ObjHashTable* ht, Vm* vm);
bool ht_set(ObjHashTable* ht, ObjString* key, Value value);
ObjString* ht_findString(ObjHashTable* ht, const char* chars, int length, uint32_t hash);
void ht_removeWhite(ObjHashTable* ht);

uint32_t fnv32(uint8_t* bytes, size_t length)
{
    uint32_t hash = FNV_OFFSET;

    for(uint8_t current_byte = 0; current_byte < length; current_byte++)
    {
        hash ^= bytes[current_byte];
        hash *= FNV_PRIME;
    }

    return hash;
}

void init_hashtable(ObjHashTable* ht, Vm* vm)
{
    ht->capacity = 0;
    ht->count = 0;
    ht->buckets = NULL;
    ht->vm = vm;
}

static Bucket* ht_find_bucket(ObjHashTable* ht, ObjString* key)
{
    uint32_t index = key->hash % ht->capacity;

    Bucket* tumba = NULL;

    while(1)
    {
        Bucket* bucket = &ht->buckets[index];

        if(bucket->key == NULL)
        {
            if(IS_NULO(bucket->value))
            {
                return tumba != NULL ? tumba : bucket;
            }
            else if (tumba == NULL)
            {
                tumba = bucket;
            }
        }
        else if(bucket->key == key)
        {
            return bucket;
        }

        index = (index + 1) % ht->capacity;
    }
}

static void ht_adjustCapacity(ObjHashTable* ht, int capacity) {
    Bucket* buckets = ALLOCATE(Bucket, capacity*sizeof(Bucket));
    Value null_value = {.type=VAL_NULO};
    for (int i = 0; i < capacity; i++) {
        buckets[i].key = NULL;
        buckets[i].value = null_value;
    }



    ht->count = 0;

    for (int i = 0; i < ht->capacity; i++) {
        Bucket* bucket = &ht->buckets[i];
        if (bucket->key == NULL) continue;

        Bucket* dest = ht_find_bucket(buckets, bucket->key);
        dest->key = bucket->key;
        dest->value = bucket->value;

        ht->count++;

    }

    FREE(Bucket, ht->buckets, ht->capacity);

    ht->buckets = buckets;
    ht->capacity = capacity;
}

bool ht_set(ObjHashTable* ht, ObjString* key, Value value)
{
    if (ht->count + 1 > ht->capacity * HTABLE_MAX) {
        int capacity = GROW_CAPACITY(ht->capacity);
        ht_adjustCapacity(ht, capacity);
    }

    uint32_t index = key->hash % ht->capacity;

    Bucket* bucket = ht_find_bucket(ht, key);

    bool isNewKey = bucket->key == NULL;

    if (isNewKey && IS_NULO(bucket->value)) ht->count++;

    bucket->key = key;
    bucket->value = value;
    return isNewKey;
}

ObjString* ht_findString(ObjHashTable* ht, const char* chars, int length, uint32_t hash)
{
    if (ht->count == 0) return NULL;

    uint32_t index = hash % ht->capacity;

    while(1)
    {
        Bucket* bucket = &ht->buckets[index];

        if(bucket->key == NULL)
        {
            if(IS_NULO(bucket->value)) return NULL;
        }
        else if(bucket->key->length == length && bucket->key->hash == hash && (memcmp(bucket->key->cstr, chars, length) == 0))
        {
            return bucket->key;
        }

        index = (index+1) % ht->capacity;
    }
}

bool ht_delete(ObjHashTable* ht, ObjString* key)
{
    if(ht->count == 0) return false;

    Bucket* bucket = ht_find_bucket(ht, key);
    if(bucket->key == NULL) return false;

    Value bool_val;
    bool_val.type = VAL_BOOL;
    bool_val.as.boolean = true;

    bucket->key = NULL;
    bucket->value = bool_val;

    return true;

}

void ht_removeWhite(ObjHashTable* ht)
{
    for(int i=0; i<ht->capacity; i++)
    {
        Bucket* bucket = &ht->buckets[i];
        if(bucket->key != NULL && !bucket->key->obj.marked)
        {
            ht_delete(ht, bucket->key);
        }
    }
}
#endif // HASH_H_INCLUDED
