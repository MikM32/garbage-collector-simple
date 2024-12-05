#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include "object.h"
#include "hash.h"


struct st_vm
{
    Value stack[1024];
    Value* stackTop;

    ObjHashTable string_table;
    ObjHashTable globals;

    Obj* heap;

    int numObj;
    int maxObj;
    int stackSize;
};

void assert(int condition, const char* message);


Value pop(Vm* vm);
void push(Vm* vm, Value value);

void assert(int condition, const char* message) {
  if (!condition) {
    printf("%s\n", message);
    exit(1);
  }
}

Obj* newObj(Vm* vm, size_t size, ObjType type)
{
    Obj* obj = ALLOCATE(Obj, size);
    assert(obj != NULL, "Runtime error: out of memory\n");
    obj->type = type;
    obj->marked = false;

    vm->numObj++;

    return obj;
}

ObjString* newObjString(Vm* vm, const char* str, int length)
{
    ObjString* obj_str = NULL;
    uint32_t hash = fnv32((uint8_t*)str, (size_t)length);


    obj_str = ht_findString(&vm->string_table, str, length, hash);

    if(obj_str != NULL) return obj_str; // La cadena ya estaba en la tabla de cadenas.

    // Se aloja la cadena dentro de la tabla de cadenas de la VM
    obj_str = (ObjString*)newObj(vm, sizeof(ObjString), OBJ_STRING);

    obj_str->hash = hash;

    obj_str->length = length;
    obj_str->obj.type = OBJ_STRING;
    obj_str->cstr = ALLOCATE(char, length+1);

    memcpy(obj_str->cstr, str, length);
    obj_str->cstr[length] = '\0';

    Value val_str, null_val;
    val_str.type = VAL_OBJ;
    val_str.as.obj = (Obj*)obj_str;
    null_val.type = VAL_NULO;

    //agrega el objeto a la lista de objetos
    obj_str->obj.next = vm->heap;
    vm->heap = (Obj*)obj_str;

    push(vm, val_str);
    ht_set(&vm->string_table, obj_str, null_val);
    pop(vm);

    return obj_str;
}

ObjArray* newObjArray(Vm* vm, int length)
{
    ObjArray* obj_array = (ObjArray*)newObj(vm, sizeof(ObjArray), OBJ_ARRAY);

    obj_array->values = realloc(NULL, length*sizeof(Value));
    obj_array->length = length;
    obj_array->obj.next = vm->heap;
    vm->heap = (Obj*)obj_array;

    return obj_array;
}


void freeObjArray(ObjArray* obj_array)
{
    free(obj_array->values);
}

void freeObjString(ObjString* obj)
{
    free(obj->cstr);
}

void init_vm(Vm* vm)
{
    vm->numObj = 0;
    vm->maxObj = 256;
    vm->stackSize = 0;
    vm->stackTop = vm->stack;

    init_hashtable(&vm->string_table, vm);
    init_hashtable(&vm->globals, vm);
    vm->heap = NULL;
}

void push(Vm* vm, Value value)
{
    assert(vm->stackSize < 1024, "Runtime error: Stack overflow\n");
    *vm->stackTop = value;
    vm->stackTop++;
    vm->stackSize++;
}

Value pop(Vm* vm)
{
    assert(vm->stackSize > 0, "Runtime error: try to pop empty stack\n");
    vm->stackSize--;
    return *(--vm->stackTop);
}

void markObject(Obj* obj)
{
    if(!obj->marked)
    {
        obj->marked = true;
    }
}

void markValue(Value* value)
{
    switch(value->type)
    {
        case VAL_OBJ:
            markObject(AS_OBJ(*value));
            break;
        default:
            break;
    }
}

void markAll(Vm* vm)
{
    for(int i = 0; i < vm->stackSize; i++)
    {
        markValue(&vm->stack[i]);
    }
}

void sweep(Vm* vm)
{
    Obj* object = vm->heap;

    while(object)
    {
        if(object->marked)
        {
            object->marked = false;
            object = object->next;
        }
        else
        {
            Obj* unreached = object;

            object = unreached->next;

            freeObj(unreached);

            vm->numObj--;
        }
    }
}

void gc(Vm* vm)
{
    int numObj = vm->numObj;

    markAll(vm);
    ht_removeWhite(&vm->string_table);
    sweep(vm);

    vm->maxObj = vm->numObj == 0 ? 256 : vm->numObj * 2;

    printf("Collected %d objects, %d remaining.\n", numObj - vm->numObj,
         vm->numObj);
}

void pushArray(Vm* vm, int length)
{
    ObjArray* array = newObjArray(vm, length);

    Value value;

    value.type = VAL_OBJ;
    AS_OBJ(value) = (Obj*)array;

    push(vm, value);
}




void freeObj(Obj* obj)
{
    switch(obj->type)
    {
        case OBJ_ARRAY:
            freeObjArray((ObjArray*)obj);
            break;
        case OBJ_HMAP: // Hashmap
            break;
        case OBJ_STRING:
            freeObjString((ObjString*)obj);
            break;
        default:
            break;
    }
}

#endif // VM_H_INCLUDED
