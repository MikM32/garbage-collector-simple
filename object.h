#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define AS_OBJ(value) ((value).as.obj)
#define IS_NULO(value) ((value).type == VAL_NULO)

typedef enum
{
    VAL_NULO=0,
    VAL_INT,
    VAL_REAL,
    VAL_BOOL,
    VAL_OBJ

}ValueType;

typedef enum
{
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_HMAP
}ObjType;

typedef struct
{
    ObjType type;
    bool marked;
    void* next;
}Obj;

typedef struct
{
    ValueType type;

    union
    {
        bool boolean;
        int  integer;
        double  real;
        Obj* obj;
    }as;

}Value;

typedef struct {

    Obj obj;
    int length;
    Value* values;
} ObjArray;



typedef struct
{
    Obj obj;
    int length;
    char* cstr;

    uint32_t hash;

} ObjString;

typedef struct st_vm Vm;

typedef struct st_bucket
{
    ObjString* key;
    Value value;
}Bucket;

typedef struct st_hashtable
{
    int capacity;
    int count;

    Vm* vm;
    Bucket* buckets;
}ObjHashTable;


#endif // OBJECT_H_INCLUDED
